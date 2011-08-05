#include <mm/pheap.h>
#include <mm/vmm.h>

static pheader_t pheap_free = 0;
static uint32_t pheap_max = PHEAP_START;

// TODO: completely wrong
void* pmalloc(uint32_t* phys) {
	if (PHEAP_END <= pheap_max)
		panic("PHEAP: out of memory.");
	uint32_t va, pa;
	if (pheap_free) {
		va = (uint32_t) pheap_free;
		pheap_free = (pheader_t) *pheap_free;
	} else {
		va = pheap_max;
		pheap_max += PAGE_SIZE;
	}
	pa = pmm_alloc_page();
	map(va, pa, PAGE_PRESENT | PAGE_WRITE);
	if (phys)
		*phys = pa;
	return (void*) va;
}

void pfree(uint32_t addr) {
	if (addr == pheap_max - PAGE_SIZE) {
		pheap_max -= PAGE_SIZE;
	} else {
		*((uint32_t*) addr) = (uint32_t) pheap_free;
		pheap_free = (pheader_t) addr;
	}
	unmap(addr);
}
