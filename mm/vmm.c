#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/pheap.h>
#include <kernel/idt.h>

extern uint8_t pmm_paging_active;

page_directory_t* current_directory;

uint32_t* tables_virtual = (uint32_t*) KERNEL_TABLES_VIRTUAL;

// note that without identity mapping this variable won't be
// valid anymore after enabling paging
static page_directory_t __directory;

void page_fault(registers_t *regs);

void init_vmm() {
	// page faults
	register_interrupt_handler(14, &page_fault);

	// FAGGOT! once again you forget about below
	// NOTE: remember that every entry in directory and tables have flags,
	// to obtain addresses must be and'ed with PAGE_MASK

	// prepare kernel page directory
	__directory.directory_physical = (uint32_t) (pmm_alloc_page() | PAGE_PRESENT
			| PAGE_WRITE);
	uint32_t* dir_ptr = (uint32_t*) (PAGE_ADDR_MASK
			& __directory.directory_physical);
	memset(dir_ptr, 0, PAGE_SIZE);

	// identity mapping for first 4 MB
	uint32_t* tab_ptr = (uint32_t*) pmm_alloc_page();
	dir_ptr[0] = (uint32_t) tab_ptr | PAGE_PRESENT | PAGE_WRITE;
	// you do KNOW what those tmp_tab's stands for
	for (int i = 0; i < 1024; i++)
		tab_ptr[i] = i * 0x1000 | PAGE_PRESENT | PAGE_WRITE;

	// mapping for directory itself
	uint16_t idx_dir = PGDIR_IDX_ADDR(KERNEL_DIR_VIRTUAL);
	uint16_t idx_tab = PGTAB_IDX_ADDR(KERNEL_DIR_VIRTUAL);
	// allocate page for directory table
	tab_ptr = (uint32_t*) pmm_alloc_page();
	dir_ptr[idx_dir] = (uint32_t) tab_ptr | PAGE_PRESENT | PAGE_WRITE;
	memset(tab_ptr, 0, PAGE_SIZE);
	// assign mapping for directory
	tab_ptr[idx_tab] = (uint32_t) dir_ptr | PAGE_PRESENT | PAGE_WRITE;

	// last table of page directory points to directory itself
	// NOTE: after that there is no need of tables_virtual, as they are
	// accessible like square table starting at KERNEL_TABLES_VIRTUAL
	dir_ptr[PGDIR_IDX_ADDR(KERNEL_TABLES_VIRTUAL)] = (uint32_t) dir_ptr
			| PAGE_PRESENT | PAGE_WRITE;

	// set the current directory
	switch_page_directory(&__directory);

	// enable paging
	asm volatile("mov %%cr0, %%eax;\
			orl $0x80000000, %%eax;\
			mov %%eax, %%cr0;"::);

	if (&__directory != current_directory)
		panic("VMM: God doesn't exist.");
	// NOTE: from that point better not use __directory -- just in case

	// find virtual address of kernel page directory
	current_directory->directory_virtual = (uint32_t*) KERNEL_DIR_VIRTUAL;

	// identity mapping pmm stack
	uint32_t idx_dir_pmm = PGDIR_IDX_ADDR(PMM_STACK_START);
	tab_ptr = (uint32_t*) pmm_alloc_page();
	current_directory->directory_virtual[idx_dir_pmm] = (uint32_t) tab_ptr
			| PAGE_PRESENT | PAGE_WRITE;
	memset(tab_ptr, 0, PAGE_SIZE);

	// wtf with those flags, not here - it's simple data
	// set up tables_virtual (rewrite mm/layout.h into an array)
	/* unnecessary since kernel knows everything about page tables (mapping)
	 current_directory->tables_virtual =
	 (uint32_t**) pmm_alloc_page();
	 for (int i = 0; i < 1024; i++)
	 current_directory->tables_virtual[i] =
	 (uint32_t*) (KERNEL_TABLES_VIRTUAL + i * PAGE_SIZE);
	 */

	// NOTE: remember that every entry in directory and tables have flags,
	// to obtain addresses must be and'ed with PAGE_MASK
	// paging active
	pmm_paging_active = 1;
}

void switch_page_directory(page_directory_t* dir) {
	current_directory = dir; // thats ok
	asm volatile("mov %0, %%cr3" : : "r" (dir->directory_physical));
}

void map(uint32_t va, uint32_t pa, uint32_t flags) {
	uint32_t idx_dir = PGDIR_IDX_ADDR(va);

	// find appropriate pagetable for va
	if (current_directory->directory_virtual[idx_dir] == 0) {
		// create pagetable holding this page
		// TODO: this code need to be revised
		// update current directory
		current_directory->directory_virtual[idx_dir] = pmm_alloc_page()
				| PAGE_PRESENT | PAGE_WRITE;
		// NEVER update kernel directory -- could screw up things
		// init page table
		memset((void*) tables_virtual[idx_dir * 1024], 0, 0x1000);
	}

	// NOTE: tables_virtual seems to be common over all page directories
	// page table exists, now update flags and pa
	tables_virtual[VTAB_IDX_ADDR(va)] = (pa & PAGE_ADDR_MASK) | flags;
}

void unmap(uint32_t va) {
	// update current directory
	tables_virtual[VTAB_IDX_ADDR(va)] = 0;
	// NEVER update kernel directory -- could screw up things
	// invalidate page mapping
	asm volatile("invlpg (%0)" : : "a" (va));
}

uint8_t get_mapping(uint32_t va, uint32_t* pa) {
	uint32_t idx_dir = PGDIR_IDX_ADDR(va);
	// Find the appropriate page table for 'va'.
	if (current_directory->directory_virtual[idx_dir] == 0)
		return 0;
	if (tables_virtual[VTAB_IDX_ADDR(va)] != 0) {
		if (pa)
			*pa = (tables_virtual[VTAB_IDX_ADDR(va)] & PAGE_ADDR_MASK)
					+ (va & PAGE_FLAGS_MASK);
		// NOTE: now it returns physical addres of variable itself, not frame
		return 1;
	}
	return 0;
}

extern void copy_page_physical(uint32_t* dest, uint32_t* src);

// TODO: do we have to update __directory too?
static uint32_t* clone_table(uint32_t* src, uint32_t* phys) {
	uint32_t* dest = (uint32_t*) pmalloc(phys);
	memset(dest, 0, PAGE_SIZE);
	for (int i = 0; i < 1024; i++) {
		if (src[i] == 0)
			continue;
		// TODO: or pmalloc?
		dest[i] = pmm_alloc_page();
		copy_page_physical((uint32_t*) dest[i],
				(uint32_t*) (PAGE_ADDR_MASK & src[i]));
		dest[i] |= (PAGE_FLAGS_MASK & src[i]);
	}
	return dest;
}

// TODO: do we have to update __directory too?
page_directory_t* clone_directory(page_directory_t* src) {
	page_directory_t* dest = kmalloc(sizeof(page_directory_t));
	dest->directory_virtual = (uint32_t*) pmalloc(&(dest->directory_physical));
	memset(dest->directory_virtual, 0, PAGE_SIZE);
	for (int i = 0; i < 1024; i++) {
		if (src->directory_virtual[i] == 0)
			continue;
		if (src->directory_virtual[i] == __directory.directory_virtual[i]) {
			dest->directory_virtual[i] = src->directory_virtual[i];
		} else {
			uint32_t phys;
			clone_table((uint32_t*) tables_virtual[i], &phys);
			dest->directory_virtual[i] = phys | PAGE_PRESENT | PAGE_WRITE
					| PAGE_USER;
		}
	}
	return dest;
}

// TODO: do we have to update __directory too?
void destroy_directory(page_directory_t* dir) {
	// TODO
}

void page_fault(registers_t *regs) {
	uint32_t cr2;
	asm volatile("mov %%cr2, %0" : "=r" (cr2));
	kprintf("Page fault: eip: 0x%x, faulting address: 0x%x\n", regs->eip, cr2);
	kprintf("Error code: %x\n", regs->err_code);
	panic("");
}
