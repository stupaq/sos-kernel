#ifndef PHEAP_H
#define PHEAP_H

#include <common.h>
#include <mm/layout.h>

typedef uint32_t* pheader_t;

void* pmalloc(uint32_t* phys);

void pfree(uint32_t virt);

#endif // PHEAP_H
