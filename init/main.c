#include <multiboot.h>
#include <common.h>
#include <monitor.h>
#include <gdt.h>
#include <idt.h>
#include <timer.h>
#include <elf.h>
#include <pmm.h>
#include <vmm.h>
#include <thread.h>
#include <lock.h>
#include <keyboard.h>
#include <initrd.h>

// defined by linker
extern uint32_t code, end;
extern void cpu_idle();

elf_t kernel_elf;
fs_node_t* kernel_initrd;

int kmain(multiboot_info_t* mboot_ptr, uint32_t kstack_ptr) {
	init_monitor();
	kprintf("kernel between 0x%.8x 0x%.8x\n", &code, &end);

	init_gdt(kstack_ptr);
	init_idt();

	init_timer(50);

	init_pmm((uint32_t) &end);
	init_vmm();

	init_heap();

	// till now we cannot allocate any page from paging stack
	pmm_collect_pages(mboot_ptr);

	kernel_elf = elf_from_multiboot(mboot_ptr);

	asm volatile("sti");

	init_scheduler(init_threading());

	init_keyboard_driver();

	kprintf("kernel mode completed.\n");

	cpu_idle();
	return 0;
}
