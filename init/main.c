#include <boot/multiboot.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/timer.h>
#include <kernel/elf.h>
#include <sched/sched.h>
#include <sched/thread.h>
#include <sched/fork.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <fs/initrd.h>
#include <fs/fs.h>
#include <common.h>
#include <monitor.h>

extern uint32_t code, end; // defined by linker
extern Elf32_Sym_Map kernel_elf; // for print stack trace;

extern void cpu_idle();
extern page_directory_t* current_directory;

// threading testing shit
#include <lock.h>
spinlock_t lock = SPINLOCK_UNLOCKED;
int fake_thread(void* arg) {
	for (;;) {
		spinlock_lock(&lock);
		kprintf(">>> %s\n", arg);
		for (int i = 0; i < 1000000; i++)
			;
		kprintf("%s >>>\n", arg);
		spinlock_unlock(&lock);
		for (int i = 0; i < 1000000; i++)
			;
	}
	return 666;
}

int kmain(multiboot_info_t* mboot_ptr, uint32_t stack_top,
		uint32_t stack_bottom) {
	// get everything what we may need from multiboot
	// kernel elf for stack tracing
	multiboot_elf_section_header_table_t* elf_sec = &mboot_ptr->u.elf_sec;
	// do we have initrd?
	multiboot_module_t* initrd_mod = 0;
	if (mboot_ptr->mods_count)
		// get the first module (initrd as we belive)
		initrd_mod = (multiboot_module_t*) mboot_ptr->mods_addr;

	// end of reserved memory (kernel, initrd)
	uint32_t pmm_start = (initrd_mod) ? initrd_mod->mod_end : (uint32_t) &end;

	// TODO: remember to set proper video mem in layout (in case of highmem)
	init_monitor();

	// setup interrupts (before first register_handler())
	init_idt();

	// enable paging (which registers interrupt handler)
	init_pmm(pmm_start);
	init_vmm();

	// after enabling paging set proper gdt
	init_gdt(stack_top);

	// and setup pit
	init_timer(1);

	// till now we cannot allocate any page from paging stack (or do any malloc)
	pmm_collect_pages(mboot_ptr);

	// now we can use both kheap and pheap
	init_kheap();

	// debug
	kprintf("kernel between 0x%.8x 0x%.8x mboot 0x%.8x\n", &code, &end,
			mboot_ptr);
	kprintf("kernel stack top 0x%.8x bottom 0x%.8x\n", stack_top,
			stack_bottom);

	// stack tracing
	kernel_elf = elf_sym_map_from_multiboot(elf_sec);
	print_stack_trace();

	// load initrd
	if (initrd_mod) {
		// set root filesystem
		fs_root = init_initrd(initrd_mod->mod_start);

		// debug
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

	// enable multitasking and multithreading
	thread_t* kernel_thread = init_threading(stack_top, stack_bottom);
	task_t* kernel_task = init_tasking(kernel_thread);
	init_scheduler(kernel_task);

	// init keyboard
	init_keyboard_driver();

	kprintf("kernel mode completed\n");

	// test threading
	//exec_thread(&fake_thread, "thread1");
	//exec_thread(&fake_thread, "thread2");

	// test forking
	exec_elf("init");
	//exec_elf("init");

	for (;;)
		monitor_put(keyboard_getchar());

	cpu_idle();
	return 0xBEEFCAFE;
}
