#include <mm/kheap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

static void alloc_chunk(uint32_t start, uint32_t len);
static void free_chunk(kheader_t* chunk);
static void split_chunk(kheader_t* chunk, uint32_t len);
static void glue_chunk(kheader_t* chunk);

static uint32_t kheap_max = KHEAP_START;
static kheader_t* kheap_first = 0;

void init_kheap() {
}

void* kmalloc(uint32_t len) {
	len += sizeof(kheader_t);

	kheader_t *cur_header = kheap_first, *prev_header = 0;
	while (cur_header) {
		if (cur_header->allocated == 0 && cur_header->length >= len) {
			split_chunk(cur_header, len);
			cur_header->allocated = 1;
			return (void*) ((uint32_t) cur_header + sizeof(kheader_t));
		}
		prev_header = cur_header;
		cur_header = cur_header->next;
	}

	uint32_t chunk_start;
	if (prev_header)
		chunk_start = (uint32_t) prev_header + prev_header->length;
	else {
		chunk_start = KHEAP_START;
		kheap_first = (kheader_t *) chunk_start;
	}

	alloc_chunk(chunk_start, len);
	cur_header = (kheader_t *) chunk_start;
	cur_header->prev = prev_header;
	cur_header->next = 0;
	cur_header->allocated = 1;
	cur_header->length = len;

	prev_header->next = cur_header;

	return (void*) (chunk_start + sizeof(kheader_t));
}

void* kmalloc_zero(uint32_t size) {
	uint32_t* addr = kmalloc(size);
	memset(addr, 0, size);
	return addr;
}

void kfree(void *p) {
	kheader_t *header = (kheader_t*) ((uint32_t) p - sizeof(kheader_t));
	header->allocated = 0;
	glue_chunk(header);
}

void alloc_chunk(uint32_t start, uint32_t len) {
	// sanity check (watch out for overflow)
	if (KHEAP_END - start < len)
		panic("Heap: out of memory.");
	while (start + len > kheap_max) {
		uint32_t page = pmm_alloc_page();
		map(kheap_max, page, PAGE_PRESENT | PAGE_WRITE);
		kheap_max += 0x1000;
	}
}

void free_chunk(kheader_t *chunk) {
	chunk->prev->next = 0;
	if (chunk->prev == 0)
		kheap_first = 0;
	// While the heap max can contract by a page and still be greater than the chunk address...
	while ((kheap_max - 0x1000) >= (uint32_t) chunk) {
		kheap_max -= 0x1000;
		uint32_t page;
		get_mapping(kheap_max, &page);
		pmm_free_page(page);
		unmap(kheap_max);
	}
}

void split_chunk(kheader_t* chunk, uint32_t len) {
	if (chunk->length - len > sizeof(kheader_t)) {
		kheader_t* newchunk = (kheader_t *) ((uint32_t) chunk + chunk->length);
		newchunk->prev = chunk;
		newchunk->next = chunk->next; //0;
		newchunk->allocated = 0;
		newchunk->length = chunk->length - len;

		chunk->next = newchunk;
		chunk->length = len;
	}
}

void glue_chunk(kheader_t *chunk) {
	if (chunk->next && chunk->next->allocated == 0) {
		chunk->length = chunk->length + chunk->next->length;
		chunk->next->next->prev = chunk;
		chunk->next = chunk->next->next;
	}
	if (chunk->prev && chunk->prev->allocated == 0) {
		chunk->prev->length = chunk->prev->length + chunk->length;
		chunk->prev->next = chunk->next;
		chunk->next->prev = chunk->prev;
		chunk = chunk->prev;
	}
	if (chunk->next == 0)
		free_chunk(chunk);
}
