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

elf_t kernel_elf;

spinlock_t kmain_lock = SPINLOCK_UNLOCKED;

int kmain(multiboot_info_t* mboot_ptr) {
	monitor_clear();

	init_gdt();
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

	for (;;)
		monitor_put(keyboard_getchar());

	return 0;
}
