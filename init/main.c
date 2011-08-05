#include <multiboot.h>
#include <common.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/timer.h>
#include <kernel/elf.h>
#include <sched/sched.h>
#include <sched/thread.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/pheap.h>
#include <fs/initrd.h>
#include <fs/fs.h>
#include <monitor.h>

extern uint32_t code, end; // defined by linker
extern elf_t kernel_elf; // for print stack trace;

extern void cpu_idle();

// threading testing shit
#include <lock.h>
spinlock_t lock = SPINLOCK_UNLOCKED;
int fake_thread(void* arg) {
	for (;;) {
		spinlock_lock(&lock);
		kprintf("entering %s\n", arg);
		for (int i = 0; i < 5000000; i++)
			;
		kprintf("leaving %s\n", arg);
		spinlock_unlock(&lock);
	}
	return 666;
}

int kmain(multiboot_info_elf_t* mboot_ptr, uint32_t kstack_addr) {
	// end of reserved memory (kernel, initrd)
	uint32_t pmm_start =
			(mboot_ptr->mods_count) ?
					*((uint32_t*) (mboot_ptr->mods_addr + 4)) : (uint32_t) &end;

	// TODO: remember to set proper video mem in layout (in case of highmem)
	init_monitor();

	// setup interrupts (before first register_handler())
	init_idt();

	// enable paging (which registers handler)
	init_pmm(pmm_start);
	init_vmm();

	// after enabling paging set proper gdt
	init_gdt(kstack_addr);

	// and setup pit
	init_timer(1);

	// till now we cannot allocate any page from paging stack (or do any malloc)
	pmm_collect_pages(mboot_ptr);

	// now we can use both kheap and pheap
	init_kheap();
	// after kheap init!
	init_pheap();

	// some debug
	kprintf("kernel between 0x%.8x 0x%.8x mboot 0x%.8x\n", &code, &end,
			mboot_ptr);

	// stack tracing
	kernel_elf = elf_from_multiboot(mboot_ptr);
	print_stack_trace();

	// load initrd
	if (mboot_ptr->mods_count) {
		// set root filesystem
		fs_root = init_initrd(*((uint32_t*) mboot_ptr->mods_addr));
		// debug
		// print content
		kprintf("initrd loaded at 0x%.8x to 0x%.8x with:\n",
				*((uint32_t*) mboot_ptr->mods_addr), pmm_start);
		dirent_t* node = 0;
		for (int i = 0; (node = readdir(fs_root, i)) != 0; i++) {
			fs_node_t* fsnode = finddir(fs_root, node->name);
			kprintf("\t%.20s\t0x%x\t%d\n", node->name, fsnode->flags,
					fsnode->length);
		}
	}

	// enable interrupts
	asm volatile("sti");

	thread_t* kernel_thread = init_threading();
	task_t* kernel_task = init_tasking(kernel_thread);
	init_scheduler(kernel_task);

	init_keyboard_driver();

	kprintf("kernel mode completed\n");

	add_thread(kernel_task, create_thread(&fake_thread, "thread1", allocate_stack(0x1000)));
	add_thread(kernel_task, create_thread(&fake_thread, "thread2", allocate_stack(0x1000)));

	for(;;)
		monitor_put(keyboard_getchar());

	cpu_idle();
	return 0xBEEFCAFE;
}
