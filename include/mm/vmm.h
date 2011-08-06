#ifndef VMM_H
#define VMM_H

#include <common.h>
#include <mm/layout.h>

#define PGDIR_IDX_PAGE(x) ((uint32_t)(x)/1024)
#define PGTAB_IDX_PAGE(x) ((uint32_t)(x)&0x3FF)

#define PGDIR_IDX_ADDR(x) ((uint32_t)((x)>>22)) // (x>>12)/1024
#define PGTAB_IDX_ADDR(x) ((uint32_t)(((x)>>12)&0x3FF)) // (x>>12)%1024

#define PAGE_SIZE 0x1000

#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER 0x4

#define PAGE_ADDR_MASK 0xFFFFF000
#define PAGE_FLAGS_MASK 0xFFF

struct page_directory {
	uint32_t directory_physical; // physical address of tables_physical (cr3)
	// these are VIRTUAL pointers (to virtual or physical addresses however)
	uint32_t* directory_virtual; // virtual pointer to directory (page tables)
	// ^ (by definition table of physical locations of pagetables)
	uint32_t** tables_virtual; // table of pointers to pagetables
};
typedef struct page_directory page_directory_t;

// Sets up the environment, page directories etc and enables paging.
void init_vmm();

// Changes address space.
void switch_page_directory(page_directory_t* pd);

// Maps the physical page "pa" into the virtual space at address "va", using 
// page protection flags "flags".
void map(uint32_t va, uint32_t pa, uint32_t flags);

// Removes one page of V->P mappings at virtual address "va".
void unmap(uint32_t va);

// Returns 1 if the given virtual address is mapped in the address space.
// If "*pa" is non-NULL, the physical address of the mapping is placed in *pa.
char get_mapping(uint32_t va, uint32_t* pa);

// Copies page directory returning it's virtual address and placing physical
// addresses in proper variables.
page_directory_t* clone_directory(page_directory_t* src);

// Deletes page directory freeing occupied memory
void destroy_directory(page_directory_t* dir);

#endif
