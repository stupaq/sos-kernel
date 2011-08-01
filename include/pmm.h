#ifndef PMM_H
#define PMM_H

#include <multiboot.h>
#include <common.h>

#define PMM_STACK_ADDR 0xFF7FF000

void init_pmm(uint32_t start);

uint32_t pmm_alloc_page();

void pmm_free_page(uint32_t p);

void pmm_collect_pages(multiboot_info_t* mboot_ptr);

#endif
