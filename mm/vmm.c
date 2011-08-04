#include <mm/vmm.h>
#include <mm/pmm.h>
#include <kernel/idt.h>

extern uint8_t pmm_paging_active;

page_directory_t* current_directory;

uint32_t* kernel_directory = (uint32_t*) KERNEL_DIR_VIRTUAL;
uint32_t* kernel_tables = (uint32_t*) KERNEL_TABLES_VIRTUAL;

// these are physical addresses, valid when properly mapped
static uint32_t pgdir_kernel[1024]__attribute__((aligned(PAGE_SIZE)));
static uint32_t pgtab_lowmem[1024]__attribute__((aligned(PAGE_SIZE)));
static uint32_t pgtab_directory[1024]__attribute__((aligned(PAGE_SIZE)));
static uint32_t pgtab_pmmstack[1024]__attribute__((aligned(PAGE_SIZE)));

void page_fault(registers_t *regs);

void init_vmm() {
	// page faults
	register_interrupt_handler(14, &page_fault);

	// NOTE: remember that every entry in directory and tables have flags,
	// to obtain addresses must be and'ed with PAGE_FLAG

	// prepare page directory
	memset(pgdir_kernel, 0, 0x1000);
	// id map for the first 4 MB
	pgdir_kernel[0] = (uint32_t) pgtab_lowmem | PAGE_PRESENT | PAGE_WRITE;
	for (int i = 0; i < 1024; i++)
		pgtab_lowmem[i] = i * 0x1000 | PAGE_PRESENT | PAGE_WRITE;

	uint16_t dir_idx = PGDIR_I_ADDR(KERNEL_DIR_VIRTUAL);
	uint16_t tab_idx = PGTAB_I_ADDR(KERNEL_DIR_VIRTUAL);

	// prepare mapping for directory itself
	pgdir_kernel[dir_idx] = (uint32_t) pgtab_directory | PAGE_PRESENT
			| PAGE_WRITE;
	memset(pgtab_directory, 0, 0x1000);

	// assign mapping for directory
	pgtab_directory[tab_idx] = (uint32_t) pgdir_kernel | PAGE_PRESENT
			| PAGE_WRITE;
	// the last table loops back on the directory itself
	pgdir_kernel[PGDIR_I_ADDR(KERNEL_TABLES_VIRTUAL)] = (uint32_t) pgdir_kernel
			| PAGE_PRESENT | PAGE_WRITE;

	// set the current directory
	switch_page_directory(pgdir_kernel);

	// enable paging
	asm volatile("mov %%cr0, %%eax;\
			orl $0x80000000, %%eax;\
			mov %%eax, %%cr0;"::);

	// identity mapping pmm stack
	uint32_t pt_idx = PGDIR_I_ADDR(PMM_STACK_START);
	kernel_directory[pt_idx] = (uint32_t) pgtab_pmmstack | PAGE_PRESENT
			| PAGE_WRITE;
	memset((void*) pgtab_pmmstack, 0, 0x1000);

	// NOTE: remember that every entry in directory and tables have flags,
	// to obtain addresses must be and'ed with PAGE_FLAG

	// paging active
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
	if (kernel_directory[pt_idx] == 0) {
		// The page table holding this page has not been created yet.
		kernel_directory[pt_idx] = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
		memset((void*) kernel_tables[pt_idx * 1024], 0, 0x1000);
	}

	// Now that the page table definately exists, we can update the PTE.
	kernel_tables[virtual_page] = (pa & PAGE_MASK) | flags;
}

void unmap(uint32_t va) {
	uint32_t virtual_page = va / 0x1000;
	kernel_tables[virtual_page] = 0;
	// invalidate page mapping
	asm volatile ("invlpg (%0)" : : "a" (va));
}

char get_mapping(uint32_t va, uint32_t *pa) {
	uint32_t virtual_page = va / 0x1000;
	uint32_t dir_idx = PAGE_DIR_IDX(virtual_page);
	// Find the appropriate page table for 'va'.
	if (kernel_directory[dir_idx] == 0)
		return 0;
	if (kernel_tables[virtual_page] != 0) {
		if (pa)
			*pa = (kernel_tables[virtual_page] & PAGE_MASK)
					+ (va & PAGE_OFF_MASK);
		// NOTE: now it returns physical addres of variable itself, not frame
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
