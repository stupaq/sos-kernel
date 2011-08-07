#include <sched/sched.h>

extern task_t* current_task;
extern thread_t* current_thread;

list_t* tasks = 0;
list_t* threads = 0;

void init_scheduler(task_t* initial_task) {
	tasks = list_new();
	list_push_back(tasks, (uint32_t*) initial_task);
	threads = initial_task->threads;
}

void add_task(task_t* new_task) {
	list_push_back(tasks, (uint32_t*) new_task);
}

// O(1) amortized, very unfair however...
void schedule() {
	if (!tasks)
		return;

	thread_t* new_thread = 0;
	do {
		// TODO: what if process dies?
		if (list_is_end(threads)) {
			// for future
			list_rewind(threads);
			// maybe reached end
			if (list_is_end(tasks))
				list_rewind(tasks);
			// choose task
			task_t* new_task = 0;
			do {
				new_task = (task_t*) list_current(tasks);
				// if has no threads - remove
				if (list_empty(new_task->threads)) {
					destroy_task(new_task);
					list_remove(tasks);
					new_task = 0;
				} else
					list_next(tasks);
			} while (!new_task);
			// roll on
			threads = new_task->threads;
			// now decide if we have to switch address space
			if (new_task != current_task) {
				// change address space (page directory)
				switch_page_directory(new_task->page_directory);
				kprintf("process: %d\n", new_task->pid);
				// below must by done! either by c or asm code
				current_task = new_task;
			}
		}

		new_thread = (thread_t*) list_current(threads);
		// remove thread if terminated
		switch (new_thread->state) {
		case THREAD_DYING:
			destroy_thread(new_thread);
			list_remove(threads);
		case THREAD_WAITING:
			new_thread = 0;
			break;
		case THREAD_RUNNING:
		default:
			list_next(threads);
			break;
		}
	} while (!new_thread);

	// switch thread if needed
	if (new_thread != current_thread) {
		switch_thread(new_thread);
		// current_thread = new_thread; // switch_task does that
	}
}
