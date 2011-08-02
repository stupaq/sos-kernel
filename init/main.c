#include <multiboot.h>
#include <common.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/timer.h>
#include <kernel/elf.h>
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <fs/initrd.h>
#include <fs/fs.h>
#include <monitor.h>

extern uint32_t code, end; // defined by linker
extern void cpu_idle();
extern elf_t kernel_elf;

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
	init_monitor();
	kprintf("kernel between 0x%.8x 0x%.8x\n", &code, &end);
	uint32_t pmm_start = (uint32_t) &end;
	if (mboot_ptr->mods_count)
		pmm_start = *((uint32_t*) (mboot_ptr->mods_addr + 4));

	init_gdt(kstack_addr);
	init_idt();

	init_timer(20);

	init_pmm(pmm_start);
	init_vmm();

	init_heap();

	// till now we cannot allocate any page from paging stack
	pmm_collect_pages(mboot_ptr);

	kernel_elf = elf_from_multiboot(mboot_ptr);
	print_stack_trace();

	if (mboot_ptr->mods_count) {
		kprintf("initrd loaded at 0x%.8x to 0x%.8x\n",
				*((uint32_t*) mboot_ptr->mods_addr), pmm_start);
		fs_root = init_initrd(*((uint32_t*) mboot_ptr->mods_addr));
		kprintf("content:\n");
		dirent_t* node = 0;
		for (int i = 0; (node = readdir(fs_root, i)) != 0; i++) {
			fs_node_t* fsnode = finddir(fs_root, node->name);
			kprintf("%.20s\t0x%x\t%d\n", node->name, fsnode->flags,
					fsnode->length);
		}
	}

	// enable interrupts
	asm volatile("sti");

	init_scheduler(init_threading());

	init_keyboard_driver();

	kprintf("kernel mode completed\n");
	// TODO: before entering user mode: you've used kmalloc several times
	// for scheduling data, that MUST be kernel-only, but currently kheap
	// allocates data on pages with PAGE_USER bit set
	// CONCLUSION: another heap

	create_thread(&fake_thread, "thread1", kmalloc(0x400)+0x400);
	create_thread(&fake_thread, "thread2", kmalloc(0x400)+0x400);

	cpu_idle();
	return 0;
}
