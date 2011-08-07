#ifndef PHEAP_H
#define PHEAP_H

#include <common.h>
#include <mm/layout.h>

typedef struct pheader {
	uint32_t addr;
	struct pheader* next;
} pheader_t;

void init_pheap();

void* pmalloc(uint32_t* phys);

void* pmalloc_zero(uint32_t* phys);

void pfree(uint32_t va);

#endif // PHEAP_H
