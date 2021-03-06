#include <kernel/gdt.h>
#include <common.h>

extern void gdt_flush(uint32_t);
extern void tss_flush();

static void gdt_set_gate(int32_t, uint32_t, uint32_t, uint8_t, uint8_t);
static void write_tss(int32_t, uint16_t, uint32_t);

gdt_entry_t gdt_entries[6];
gdt_ptr_t gdt_ptr;
tss_entry_t tss_entry;

void init_gdt(uint32_t kstack_ptr) {
	// We have six entries in the GDT - two for kernel mode,
	// two for user mode, the NULL descriptor,
	// and one for the TSS (task state segment)
	// The limit is the last valid byte from the start of the GDT
	// - i.e. the size of the GDT - 1.
	gdt_ptr.limit = sizeof(gdt_entry_t) * 6 - 1;
	gdt_ptr.base = (uint32_t) &gdt_entries;

	gdt_set_gate(0, 0, 0, 0, 0); // null segment
	gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF); // kernel code
	gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF); // kernel data
	gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0xCF); // user code
	gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0xCF); // user data
	// we pass gdt_entry index, kernel stack data segment offset
	// and kernel stack pointer
	write_tss(5, 0x10, kstack_ptr);

	gdt_flush((uint32_t) &gdt_ptr);
	tss_flush();
}

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit,
		uint8_t access, uint8_t gran) {
	gdt_entries[num].base_low = (base & 0xFFFF);
	gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	gdt_entries[num].base_high = (base >> 24) & 0xFF;

	gdt_entries[num].limit_low = (limit & 0xFFFF);
	gdt_entries[num].granularity = (limit >> 16) & 0x0F;

	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access = access;
}

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
	// Firstly, let's compute the base and limit of our entry into the GDT.
	uint32_t base = (uint32_t) &tss_entry;
	uint32_t limit = base + sizeof(tss_entry);

	// Now, add our TSS descriptor's address to the GDT.
	gdt_set_gate(num, base, limit, 0xE9, 0x00);

	// Ensure the descriptor is initially zero.
	memset(&tss_entry, 0, sizeof(tss_entry));

	tss_entry.ss0 = ss0; // set the kernel stack segment
	tss_entry.esp0 = esp0; // set the kernel stack pointer

	// Here we set the cs, ss, ds, es, fs and gs entries in the TSS.
	// These specify what segments should be loaded when the processor
	// switches to kernel mode. Therefore they are just our normal
	// kernel code/data segments - 0x08 and 0x10 respectively,
	// but with the last two bits set, making 0x0b and 0x13.
	// The setting of these bits sets the RPL (requested privilege level)
	// to 3, meaning that this TSS can be used to switch to
	// kernel mode from ring 3.
	tss_entry.cs = 0x0b;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs =
			0x13;
}
