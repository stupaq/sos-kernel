#include <mm/vmm.h>
#include <mm/pmm.h>
#include <kernel/idt.h>

extern uint8_t pmm_paging_active;

page_directory_t* current_directory;

uint32_t* kpage_directory = (uint32_t*) PAGE_DIR_VIRTUAL_ADDR;
uint32_t* kpage_tables = (uint32_t*) PAGE_TABLE_VIRTUAL_ADDR;

void page_fault(registers_t *regs);

void init_vmm() {
	int i;
	uint32_t cr0;

	register_interrupt_handler(14, &page_fault);

	page_directory_t *new_directory = (page_directory_t*) pmm_alloc_page();
	memset(new_directory, 0, 0x1000);

	// id map for the first 4 MB
	new_directory[0] = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
	uint32_t* tab = (uint32_t*) (new_directory[0] & PAGE_MASK);
	for (i = 0; i < 1024; i++)
		tab[i] = i * 0x1000 | PAGE_PRESENT | PAGE_WRITE;

	uint16_t dir_idir = PGDIR_I_ADDR(PAGE_DIR_VIRTUAL_ADDR);
	uint16_t dir_itab = PGTAB_I_ADDR(PAGE_DIR_VIRTUAL_ADDR);
	// prepare mapping for directory itself
	new_directory[dir_idir] = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
	tab = (uint32_t*) (new_directory[dir_idir] & PAGE_MASK);
	memset(tab, 0, 0x1000);

	// assign mapping for directory
	tab[dir_itab] = (uint32_t) new_directory | PAGE_PRESENT | PAGE_WRITE;
	// The last table loops back on the directory itself.
	new_directory[PGDIR_I_ADDR(PAGE_TABLE_VIRTUAL_ADDR)] =
			(uint32_t) new_directory | PAGE_PRESENT | PAGE_WRITE;

	// set the current directory.
	switch_page_directory(new_directory);

	// enable paging
	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));

	// identity mapping pmm stack
	uint32_t pt_idx = PGDIR_I_ADDR(PMM_STACK_ADDR);
	kpage_directory[pt_idx] = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
	// NOTE: there are no tables at this directory
	memset((void*) kpage_directory[pt_idx], 0, 0x1000);

	// Paging is now active. Tell the physical memory manager.
	pmm_paging_active = 1;

}

void switch_page_directory(page_directory_t* pd) {
	current_directory = pd;
	asm volatile ("mov %0, %%cr3" : : "r" (pd));
}

void map(uint32_t va, uint32_t pa, uint32_t flags) {
	uint32_t virtual_page = va / 0x1000;
	uint32_t pt_idx = PAGE_DIR_IDX(virtual_page);

	// Find the appropriate page table for 'va'.
	if (kpage_directory[pt_idx] == 0) {
		if (va == 0xff7ffffc) panic("break");
		// The page table holding this page has not been created yet.
		kpage_directory[pt_idx] = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
		memset((void*) kpage_tables[pt_idx * 1024], 0, 0x1000);
	}

	// Now that the page table definately exists, we can update the PTE.
	kpage_tables[virtual_page] = (pa & PAGE_MASK) | flags;
}

void unmap(uint32_t va) {
	uint32_t virtual_page = va / 0x1000;
	kpage_tables[virtual_page] = 0;
	// invalidate page mapping
	asm volatile ("invlpg (%0)" : : "a" (va));
}

char get_mapping(uint32_t va, uint32_t *pa) {
	uint32_t virtual_page = va / 0x1000;
	uint32_t pt_idx = PAGE_DIR_IDX(virtual_page);
	// Find the appropriate page table for 'va'.
	if (kpage_directory[pt_idx] == 0)
		return 0;

	if (kpage_tables[virtual_page] != 0) {
		if (pa)
			*pa = kpage_tables[virtual_page] & PAGE_MASK;
		return 1;
	}
	return 0;
}

void page_fault(registers_t *regs) {
	uint32_t cr2;
	asm volatile ("mov %%cr2, %0" : "=r" (cr2));
	kprintf("Page fault at 0x%x, faulting address 0x%x\n", regs->eip, cr2);
	kprintf("Error code: %x\n", regs->err_code);
	panic("");
}
