#include <boot/multiboot.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/timer.h>
#include <kernel/elf.h>
#include <kernel/kbd.h>
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

int init();

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
	kbd_init_driver();

	debug_checkpoint("kernel ready");

	// enable multitasking and multithreading
	thread_t* kernel_thread = init_threading(stack_top, stack_bottom);
	task_t* kernel_task = init_tasking(kernel_thread);
	init_scheduler(kernel_task);

	debug_checkpoint("sheduler working");

	exec_elf("user/init");
	debug_checkpoint("spawned process init");

	exec_thread((int(*)(void*)) init, 0);
	debug_checkpoint("spawned thread init");

	for (;;)
		monitor_put(kbd_getchar());

	cpu_idle();
	return 0xBEEFCAFE;
}

int init() {
	debug_checkpoint("entering init thread");
	/*// debug
	debug_register_handler(6);
	uint32_t pid = fork();
	kprintf("%d\n", pid);
	*/
	debug_checkpoint("leaving init thread");
	return 0;
}
