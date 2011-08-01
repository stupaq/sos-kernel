#include <pmm.h>
#include <vmm.h>

uint8_t pmm_paging_active = 0;

static uint32_t pmm_stack_loc = PMM_STACK_ADDR;
static uint32_t pmm_stack_max = PMM_STACK_ADDR;
static uint32_t pmm_location;

void init_pmm(uint32_t start) {
	// Ensure the initial page allocation location is page-aligned.
	pmm_location = (start + 0x1000) & PAGE_MASK;
}

uint32_t pmm_alloc_page() {
	if (pmm_paging_active) {
		// sanity check
		if (PMM_STACK_ADDR == pmm_stack_loc)
			panic("PMM Stack: out of memory.");

		// pop
		pmm_stack_loc -= sizeof(uint32_t);
		uint32_t* stack = (uint32_t*) pmm_stack_loc;

		// NOTE: pmm_stack_loc is a pointer to a pointer TO the free page,
		// that is also the top of the stack, so we return VALUE
		// at the stack's top, not pointer to it.
		return *stack;
	} else {
		return pmm_location += 0x1000;
	}
}

void pmm_free_page(uint32_t p) {
	// Ignore any page under "location", as it may contain important
	// data initialised at boot (like paging structures!)
	if (p < pmm_location)
		return;

	// If we've run out of space on the stack...
	if (pmm_stack_max <= pmm_stack_loc) {
		// sanity check
		if (PAGE_DIR_VIRTUAL_ADDR <= pmm_stack_max)
			panic("PMM Stack: reached page directory addresses.");
		// Map the page we're currently freeing at the top of the free page stack.
		map(pmm_stack_max, p, PAGE_PRESENT | PAGE_WRITE);
		// Increase the free page stack's size by one page.
		pmm_stack_max += 4096;
	} else {
		// Else we have space on the stack, so push.
		uint32_t* stack = (uint32_t*) pmm_stack_loc;
		*stack = p;
		pmm_stack_loc += sizeof(uint32_t);
	}
}

void pmm_collect_pages(multiboot_info_t* mboot_ptr) {
	uint32_t i = mboot_ptr->mmap_addr;
	while (i < mboot_ptr->mmap_addr + mboot_ptr->mmap_length) {
		multiboot_memory_map_t *me = (multiboot_memory_map_t*) i;
		// Does this entry specify usable RAM?
		if (me->type == 1) {
			uint32_t j;
			// For every page in this entry, add to the free page stack.
			for (j = me->base_addr_low; j < me->base_addr_low + me->length_low;
					j += 0x1000) {
				pmm_free_page(j);
			}
		}
		// The multiboot specification is strange in this respect:
		// the size member does not include "size" itself in its calculations,
		// so we must add sizeof (uint32_t).
		i += me->size + sizeof(uint32_t);
	}
}
