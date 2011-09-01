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
#include <debug.h>

extern uint32_t code, end; // defined by linker
extern Elf32_Sym_Map kernel_elf; // for print stack trace;

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
	init_timer(25);

	// till now we cannot allocate any page from paging stack (or do any malloc)
	pmm_collect_pages(mboot_ptr);

	// now we can use both kheap and pheap
	init_kheap();

	debug_print_kernel_info();

	// stack tracing
	kernel_elf = elf_sym_map_from_multiboot(elf_sec);
	print_stack_trace();

	// load initrd
	if (initrd_mod) {
		// set root filesystem
		fs_root = init_initrd(initrd_mod->mod_start);

		debug_print_initrd_info();
		debug_print_root_content(fs_root);
	}

	// enable interrupts
	asm volatile("sti");

	// init keyboard
	init_keyboard_driver();

	debug_checkpoint("kernel ready");

	// enable multitasking and multithreading
	thread_t* kernel_thread = init_threading(stack_top, stack_bottom);
	task_t* kernel_task = init_tasking(kernel_thread);
	init_scheduler(kernel_task);

	// test threading
	//debug_run_threads(3);

	// test forking
	exec_elf("init");

	for (;;)
		monitor_put(keyboard_getchar());

	cpu_idle();
	return 0xBEEFCAFE;
}
