#include <multiboot.h>
#include <common.h>
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

spinlock_t lock = SPINLOCK_UNLOCKED;

int kmain(multiboot_info_t *mboot_ptr) {
	monitor_clear();

	init_gdt();
	init_idt();
	kprintf("descriptor tables.\n");

	init_timer(50);
	kprintf("timer\n");

	init_pmm(mboot_ptr->mem_upper);
	init_vmm();
	init_heap();
	kprintf("mm & heap\n");

	// Find all the usable areas of memory and inform
	// the physical memory manager about them.
	uint32_t i = mboot_ptr->mmap_addr;
	while (i < mboot_ptr->mmap_addr + mboot_ptr->mmap_length) {
		multiboot_memory_map_t *me = (multiboot_memory_map_t*) i;

		// Does this entry specify usable RAM?
		if (me->type == 1) {
			uint32_t j;
			// For every page in this entry, add to the free page stack.
			for (j = me->base_addr_low; j < me->base_addr_low + me->length_low;
					j += 0x1000) {
				pmm_free_page(j);
			}
		}
		// The multiboot specification is strange in this respect:
		// the size member does not include "size" itself in its calculations,
		// so we must add sizeof (uint32_t).
		i += me->size + sizeof(uint32_t);
	}
	kprintf("physical memory mapping\n");

	kernel_elf = elf_from_multiboot(mboot_ptr);
	kprintf("kernel elf\n");

	asm volatile("sti");
	kprintf("interrupts enabled\n");

	init_scheduler(init_threading());
	kprintf("multithreading\n");

	init_keyboard_driver();
	kprintf("keyboard driver\n");

	for (;;)
		monitor_put(keyboard_getchar());

	return 0;
}
