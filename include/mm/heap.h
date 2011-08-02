#ifndef HEAP_H
#define HEAP_H

#include <mm/pmm.h>
#include <common.h>

#define HEAP_START 0xD0000000
#define HEAP_END PMM_STACK_ADDR
// TODO: this was a mistake, because at 0xFF000000 is paging stack
//#define HEAP_END   0xFFBFF000

typedef struct header {
	struct header *prev, *next;
	uint32_t allocated :1;
	uint32_t length :31;
} header_t;

void init_heap();

// Returns a pointer to a chunk of memory, minimum size 'l' bytes.
void* kmalloc(uint32_t l);

// Takes a chunk of memory allocated with kmalloc, and returns it to the pool of usable memory.
void kfree(void *p);

#endif
