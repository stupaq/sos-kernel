#ifndef GDT_H
#define GDT_H

#include <common.h>

// Initialisation function.
void init_gdt();

typedef struct {
	uint16_t limit_low; // The lower 16 bits of the limit.
	uint16_t base_low; // The lower 16 bits of the base.
	uint8_t base_middle; // The next 8 bits of the base.
	uint8_t access; // Access flags, determines what ring this segment can be used in.
	uint8_t granularity;
	uint8_t base_high; // The last 8 bits of the base.
}__attribute__((packed)) gdt_entry_t;

// This struct describes a GDT pointer. It points to the start of
// our array of GDT entries, and is in the format required by the
// lgdt instruction.
typedef struct {
	uint16_t limit; // The Global Descriptor Table limit.
	uint32_t base; // The address of the first gdt_entry_t struct.
}__attribute__((packed)) gdt_ptr_t;

#endif

