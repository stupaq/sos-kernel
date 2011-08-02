#include <panic.h>
#include <common.h>
#include <elf.h>

extern void system_hung();
extern elf_t kernel_elf;

static void print_stack_trace();

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
		kprintf("   [0x%x] %s\n", *eip, elf_lookup_symbol(*eip, &kernel_elf));
		ebp = (uint32_t*) *ebp;
	}
}
