#ifndef PMM_H
#define PMM_H

#include <boot/multiboot.h>
#include <mm/layout.h>
#include <common.h>

void init_pmm(uint32_t pmm_start);

uint32_t pmm_alloc_page();

void pmm_free_page(uint32_t p);

void pmm_collect_pages(multiboot_info_t* mboot_ptr);

#endif
