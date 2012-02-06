#include <sched/thread.h>
#include <mm/vmm.h>

// sched.c and thread.s relie on that
thread_t* current_thread = 0;
uint32_t next_tid = 0;

thread_t* kernel_thread = 0;

void thread_exit();

thread_t* init_threading(uint32_t kstack_top, uint32_t kstack_bottom) {
	current_thread = kernel_thread = kmalloc_zero(sizeof(thread_t));
	current_thread->tid = next_tid++;
	current_thread->state = THREAD_RUNNING;
	current_thread->stack_size = kstack_top - kstack_bottom + 4; // a word
	current_thread->esp0 = kstack_top;
	return current_thread;
}

uint32_t* allocate_stack(uint32_t size) {
	uint32_t va = find_free_range(USER_STACKS_START, USER_STACKS_END, size, 1);

	if (va == USER_STACKS_END)
		panic("No place for stacks.");

	allocate_range(va, va + size, PAGE_USER | PAGE_WRITE);
	return (uint32_t*) (va + size);
}

thread_t* create_thread(int(*fn)(void*), void* arg, uint32_t* stack,
		uint32_t stack_size) {
	thread_t* thread = kmalloc_zero(sizeof(thread_t));
	thread->tid = next_tid++;
	thread->state = THREAD_RUNNING;
	thread->stack_size = stack_size;
	thread->esp0 = (uint32_t) stack;

	// __cdecl convetion and o at the bottom to help stack tracing
	*--stack = 0;
	*--stack = (uint32_t) arg;
	*--stack = (uint32_t) &thread_exit;
	// next return (from scheduling function will enter fn)
	*--stack = (uint32_t) fn;

	thread->esp = (uint32_t) stack;
	thread->ebp = 0;
	thread->eflags = 0x200; // enabling interrupts on ret

	return thread;
}

void destroy_thread(thread_t* thread) {
	kfree(thread);
}

void thread_exit() {
	register uint32_t val asm("eax");
	current_thread->state = THREAD_DYING;
	kprintf("thread exited with value %d\n", val);
	for (;;)
		;
}
