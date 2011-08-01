#include <multiboot.h>
#include <monitor.h>
#include <gdt.h>
#include <idt.h>
#include <timer.h>
#include <elf.h>
#include <pmm.h>
#include <vmm.h>
#include <heap.h>
#include <thread.h>
#include <lock.h>
#include <keyboard.h>
#include <initrd.h>

extern void cpu_idle();

void kinit();

elf_t kernel_elf;
fs_node_t* kernel_initrd;

int kmain(multiboot_info_t* mboot_ptr, uint32_t kstack_ptr) {
	monitor_clear();

	init_gdt(kstack_ptr);
	init_idt();

	init_timer(50);

	init_pmm(mboot_ptr->mem_upper);
	init_vmm();

	init_heap();

	// till now we cannot allocate any page from paging stack
	pmm_collect_pages(mboot_ptr);

	kernel_elf = elf_from_multiboot(mboot_ptr);

	asm volatile("sti");

	init_scheduler(init_threading());

	init_keyboard_driver();

	monitor_write("kernel mode completed.\n");

	for (;;)
		monitor_put(keyboard_getchar());

	cpu_idle();
	return 0;
}

void kinit() {
}
