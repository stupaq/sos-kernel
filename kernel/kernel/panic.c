#include <kernel/panic.h>
#include <kernel/elf.h>
#include <common.h>

Elf32_Sym_Map kernel_elf;

static void system_hung() {
	asm volatile("cli \n\
			hlt \n\
			ret");
}

void panic(const char *msg) {
	kprintf("*** System panic: %s\n", msg);
	print_stack_trace();
	kprintf("***\n");
	system_hung();
}

void print_stack_trace() {
	uint32_t *ebp, *eip;
	asm volatile ("mov %%ebp, %0" : "=r" (ebp));
	while (ebp) {
		eip = ebp + 1;
		kprintf("\t[0x%x] %s\n", *eip, elf_sym_map_lookup(*eip, &kernel_elf));
		ebp = (uint32_t*) *ebp;
	}
}
