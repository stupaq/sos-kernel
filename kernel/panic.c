#include <kernel/panic.h>
#include <kernel/elf.h>
#include <common.h>
#include <kprintf.h>

extern void system_hung();

elf_t kernel_elf;

void print_stack_trace() {
	uint32_t *ebp, *eip;
	asm volatile ("mov %%ebp, %0" : "=r" (ebp));
	while (ebp) {
		eip = ebp + 1;
		kprintf("\t[0x%x] %s\n", *eip, elf_lookup_symbol(*eip, &kernel_elf));
		ebp = (uint32_t*) *ebp;
	}
}

void panic(const char *msg) {
	kprintf("*** System panic: %s\n", msg);
	print_stack_trace();
	kprintf("***\n");
	system_hung();
}
