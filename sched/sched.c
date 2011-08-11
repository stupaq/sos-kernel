#include <sched/sched.h>

extern task_t* current_task;
extern thread_t* current_thread;

list_t* tasks = 0;
list_t* threads = 0;

void init_scheduler(task_t* initial_task) {
	tasks = list_new();
	list_push_back(tasks, (uint32_t*) initial_task);
	threads = initial_task->threads;
	list_next(threads);
}

void sched_add_task(task_t* new_task) {
	list_push_back(tasks, (uint32_t*) new_task);
}

static uint32_t tick = 0;

// O(1) amortized, very unfair however...
void schedule() {
	if (!tasks)
		return;

	tick++;
	if (tick % 100)
		return;

	if (tick > 300 && current_task->pid > 0) {
		kprintf("killing tid: 1.\n");
		current_thread->state = THREAD_DYING;
	}

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
				// rewind if necessary
				if (list_is_end(tasks))
					list_rewind(tasks);
				// pick task
				new_task = (task_t*) list_current(tasks);
				// if has no threads - remove
				if (list_empty(new_task->threads)) {
					kprintf("delete pid: %d\n", new_thread->tid);
					list_remove(tasks);
					destroy_task(new_task);
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
				// below must by done! either by c or asm code
				current_task = new_task;
			}
		}

		new_thread = (thread_t*) list_current(threads);
		// remove thread if terminated
		switch (new_thread->state) {
		case THREAD_DYING:
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

	// switch thread if needed
	if (new_thread != current_thread) {
		//switch_thread(new_thread);
		// debug
		current_thread = new_thread; // switch_thread does that
		kprintf("pid: %d tid: %d\n\n", current_task->pid, current_thread->tid);
	}
}
