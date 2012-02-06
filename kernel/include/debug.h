#ifndef DEBUG_H
#define DEBUG_H

#include <common.h>
#include <fs/fs.h>

#define debug_print_kernel_info() \
	kprintf("kernel between 0x%.8x 0x%.8x mboot 0x%.8x\n", &code, &end, \
			mboot_ptr); \
	kprintf("kernel stack top 0x%.8x bottom 0x%.8x\n", stack_top, \
			stack_bottom); \

#define debug_print_initrd_info() \
	kprintf("initrd loaded at 0x%.8x to 0x%.8x with:\n", \
			*((uint32_t*) mboot_ptr->mods_addr), pmm_start); \

#define debug_checkpoint(str) \
	kprintf("checkpoint: %s\n", str);

void debug_print_root_content(fs_node_t* fs_root);

void debug_run_threads(uint32_t num);

void debug_register_handler(uint8_t n);

#endif // DEBUG_H
