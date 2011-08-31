#include <sched/sched.h>
#include <sched/thread.h>

extern task_t* current_task;
extern thread_t* kernel_thread;

list_t* tasks = 0;
list_t* threads = 0;

// this stack exists in kernel address space so is accessible
// in all page directories
static uint32_t scheduler_stack = 0;

void init_scheduler(task_t* initial_task) {
	// allocate stack for scheduler operations
	scheduler_stack = (uint32_t) allocate_stack(THREAD_STACK_SIZE);
	// prepare initial task queue
	tasks = list_new();
	list_push_back(tasks, (uint32_t*) initial_task);
	threads = initial_task->threads;
	list_next(threads);
}

void schedule_add_task(task_t* new_task) {
	list_push_back(tasks, (uint32_t*) new_task);
}

// O(1) amortized, very unfair however...
void schedule() {
	if (!tasks)
		return;

	// switch to kernel thread
	switch_thread(kernel_thread);

	task_t* new_task = 0;
	thread_t* new_thread = 0;
	do {
		if (list_is_end(threads)) {
			// for future
			list_rewind(threads);
			// choose task
			do {
				// rewind if necessary
				if (list_is_end(tasks))
					list_rewind(tasks);
				// pick task
				new_task = (task_t*) list_current(tasks);
				// if has no threads - remove
				if (list_is_empty(new_task->threads)) {
					// debug
					kprintf("delete pid: %d\n", new_task->pid);
					list_remove(tasks);
					destroy_task(new_task);
					new_task = 0;
				} else
					list_next(tasks);
			} while (!new_task);
			// roll on
			threads = new_task->threads;
			// done already but just in case
			list_rewind(threads);
			// now decide if we have to switch address space
			if (new_task != current_task) {
				// change task
				current_task = new_task;
				// change address space (page directory)
				switch_page_directory(new_task->page_directory);
			}
		}

		new_thread = (thread_t*) list_current(threads);
		// remove thread if terminated
		switch (new_thread->state) {
		case THREAD_DYING:
			// debug
			kprintf("delete tid: %d\n", new_thread->tid);
			list_remove(threads);
			destroy_thread(new_thread);
			new_thread = 0;
			break;
		case THREAD_WAITING:
			new_thread = 0;
			break;
		case THREAD_RUNNING:
		default:
			list_next(threads);
			break;
		}
	} while (!new_thread);

	// switch_thread changes current_thread too
	switch_thread(new_thread);
}
