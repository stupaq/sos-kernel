#include <mm/pheap.h>
#include <mm/vmm.h>

static pheader_t* pheap_free = 0;
static uint32_t pheap_max = PHEAP_START;

void init_pheap() {
}

void* pmalloc(uint32_t* phys) {
	if (PHEAP_END <= pheap_max)
		panic("PHEAP: out of memory.");
	uint32_t va, pa;
	if (pheap_free) {
		pheader_t* header = pheap_free;
		va = header->addr;
		pheap_free = header->next;
		kfree(header);
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

void* pmalloc_zero(uint32_t* phys) {
	uint32_t* addr = pmalloc(phys);
	memset(addr, 0, PAGE_SIZE);
	return addr;
}

void pfree(uint32_t va) {
	if (va + PAGE_SIZE == pheap_max) {
		pheap_max -= PAGE_SIZE;
	} else {
		pheader_t* header = kmalloc(sizeof(pheader_t));
		header->addr = va;
		header->next = pheap_free;
		pheap_free = header;
	}
	unmap(va);
}
