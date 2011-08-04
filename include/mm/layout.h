#ifndef LAYOUT_H
#define LAYOUT_H

// no effect
#define LOWMEM_ID_MAP_START		0x00000000
#define LOWMEM_ID_MAP_END		0x00400000

#define VIDEO_MEM				0x000B8000

#define HEAP_START				0xD0000000
#define HEAP_END				0xFF000000

// no effect (and currently kernel resides in low mem)
#define KERNEL_HIGHMEM			0xFF000000
#define KERNEL_HIGHMEM_LIMIT	0xFF7FF000

#define PMM_STACK_START			0xFF7FF000
#define PMM_STACK_END			0xFFBFF000

#define KERNEL_DIR_VIRTUAL		0xFFBFF000
#define KERNEL_TABLES_VIRTUAL	0xFFC00000

#endif // LAYOUT_H
