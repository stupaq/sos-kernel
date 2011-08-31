#include <kernel/timer.h>
#include <kernel/idt.h>
#include <sched/sched.h>
#include <common.h>

uint32_t tick = 0;
uint32_t frequency = 0;

static void timer_callback(registers_t *regs) {
	tick++;
	schedule();
}

void init_timer(uint32_t freq) {
	// register timer callback
	register_interrupt_handler(IRQ0, &timer_callback);

	// NOTE: the divisor must be small enough to fit into 16-bits.
	uint32_t divisor = TIMER_FREQUENCY / freq;

	// save frequency
	frequency = TIMER_FREQUENCY / divisor;

	// send the command byte
	outb(0x43, 0x36);

	// Divisor has to be sent byte-wise, so split here into upper/lower bytes.
	uint8_t l = (uint8_t) (divisor & 0xFF);
	uint8_t h = (uint8_t) ((divisor >> 8) & 0xFF );

	// send the frequency divisor
	outb(0x40, l);
	outb(0x40, h);
}
