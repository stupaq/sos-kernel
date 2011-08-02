#include <multiboot.h>
#include <common.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/timer.h>
#include <kernel/elf.h>
#include <kernel/keyboard.h>
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <fs/initrd.h>
#include <fs/fs.h>
#include <monitor.h>
#include <kprintf.h>

// defined by linker
extern uint32_t code, end;

extern void cpu_idle();
elf_t kernel_elf;

int kmain(multiboot_info_elf_t* mboot_ptr, uint32_t kstack_ptr) {
	init_monitor();
	kprintf("kernel between 0x%.8x 0x%.8x\n", &code, &end);
	uint32_t pmm_start = (uint32_t) &end;
	if (mboot_ptr->mods_count)
		pmm_start = *((uint32_t*) (mboot_ptr->mods_addr + 4));

	init_gdt(kstack_ptr);
	init_idt();

	init_timer(50);

	init_pmm(pmm_start);
	init_vmm();

	init_heap();

	// till now we cannot allocate any page from paging stack
	pmm_collect_pages(mboot_ptr);

	kernel_elf = elf_from_multiboot(mboot_ptr);

	if (mboot_ptr->mods_count) {
		uint32_t initrd_loc = *((uint32_t*) mboot_ptr->mods_addr);
		kprintf("initrd loaded at 0x%.8x to 0x%.8x\n", initrd_loc, pmm_start);
		fs_node_t* initrd = init_initrd(initrd_loc);
		kprintf("content:\n");
		struct dirent* node = 0;
		for (int i = 0; (node = fs_node_readdir(initrd, i)) != 0; i++) {
			fs_node_t* fsnode = fs_node_finddir(initrd, node->name);
			kprintf("%.20s\t0x%x\t%d\n", node->name, fsnode->flags,
					fsnode->length);
		}
	}

	asm volatile("sti");

	init_scheduler(init_threading());

	init_keyboard_driver();

	//panic("kernel mode completed");
	cpu_idle();
	return 0;
}
