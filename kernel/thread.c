#include <kernel/thread.h>
#include <kernel/scheduler.h>
#include <mm/heap.h>

thread_t *current_thread;
uint32_t next_tid = 0;

void thread_exit();

thread_t* init_threading() {
	thread_t* thread = kmalloc(sizeof(thread_t));
	thread->id = next_tid++;
	current_thread = thread;
	return thread;
}

thread_t* create_thread(int(*fn)(void*), void* arg, uint32_t* stack) {
	thread_t* thread = kmalloc(sizeof(thread_t));
	memset(thread, 0, sizeof(thread_t));
	thread->id = next_tid++;

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
