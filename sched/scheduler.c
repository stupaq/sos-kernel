#include <sched/scheduler.h>

thread_t* ready_queue = 0;
thread_t* current_thread = 0;

void init_scheduler(thread_t* initial_thread) {
	current_thread = initial_thread;
	current_thread->next = 0;
	ready_queue = 0;
}

void thread_is_ready(thread_t* t) {
	t->next = 0;
	if (!ready_queue) {
		// ready queue is empty
		ready_queue = t;
	} else {
		// iterate through the ready queue to the end
		thread_t* iterator = ready_queue;
		while (iterator->next)
			iterator = iterator->next;
		// add at the end
		iterator->next = t;
	}
}

void thread_not_ready(thread_t* t) {
	// find the thread in the ready queue
	thread_t* iterator = ready_queue;
	// thread is first in the queue
	if (iterator == t) { // it's fine - pointers should be the same
		ready_queue = iterator->next;
		//kfree(iterator); just remove from queue
		return;
	}
	while (iterator->next) {
		if (iterator->next == t) {
			//thread_t* tmp = iterator->next;
			iterator->next = iterator->next->next;
			//kfree(tmp); just remove from queue
			return;
		}
		iterator = iterator->next;
	}
}

// TODO: it's O(n) - no way!
void schedule() {
	if (!ready_queue)
		return;

	// iterate through
	thread_t* iterator = ready_queue;
	while (iterator->next)
		iterator = iterator->next;

	// add current (old) to the end and remove from start
	iterator->next = current_thread;
	current_thread->next = 0;
	thread_t* new_thread = ready_queue;
	ready_queue = ready_queue->next;

	// switch
	switch_thread(new_thread);
}
