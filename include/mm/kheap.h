#ifndef HEAP_H
#define HEAP_H

#include <common.h>
#include <mm/pmm.h>

typedef struct kheader {
	struct kheader* prev,* next;
	uint32_t allocated :1;
	uint32_t length :31;
} kheader_t;

void init_kheap();

// Returns a pointer to a chunk of memory, minimum size 'l' bytes.
void* kmalloc(uint32_t l);

// Takes a chunk of memory allocated with kmalloc, and returns it to the pool of usable memory.
void kfree(void *p);

#endif
