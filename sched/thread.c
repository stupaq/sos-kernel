#include <sched/thread.h>
#include <sched/scheduler.h>

thread_t *current_thread;
uint32_t next_tid = 0;

void thread_exit();

thread_t* init_threading() {
	thread_t* thread = kmalloc(sizeof(thread_t));
	thread->tid = next_tid++;
	current_thread = thread;
	return thread;
}

uint32_t* allocate_stack(uint32_t size) {
	// offset of size * sizof(uint8_t) / sizeof(uint32_t)
	return (uint32_t*) kmalloc(size) + size / 4;
}

thread_t* create_thread(int(*fn)(void*), void* arg, uint32_t* stack) {
	thread_t* thread = kmalloc(sizeof(thread_t));
	memset(thread, 0, sizeof(thread_t));
	thread->tid = next_tid++;

	*--stack = (uint32_t) arg;
	*--stack = (uint32_t) &thread_exit;
	*--stack = (uint32_t) fn;

	thread->esp = (uint32_t) stack;
	thread->ebp = 0;
	thread->eflags = 0x200; // enabling interrupts on ret
	thread_is_ready(thread);

	return thread;
}

void thread_exit() {
	register uint32_t val asm("eax");
	kprintf("Thread exited with value %d\n", val);
	for (;;)
		;
}
