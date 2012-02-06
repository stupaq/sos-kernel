#include <sched/task.h>

// sched.c and task.s relie on that
task_t* current_task = 0;
uint32_t next_pid = 0;

task_t* kernel_task = 0;

extern page_directory_t* current_directory;

task_t* init_tasking(thread_t* init_thread) {
	current_task = kernel_task = create_task(init_thread, current_directory);
	return current_task;
}

task_t* create_task(thread_t* main_thread, page_directory_t* directory) {
	task_t* task = kmalloc_zero(sizeof(task_t));
	task->page_directory = directory;
	task->pid = next_pid++;
	task->threads = list_new();
	list_push_back(task->threads, (uint32_t*) main_thread);
	return task;
}

void destroy_task(task_t* task) {
	list_rewind(task->threads);
	while(!list_is_end(task->threads))
		destroy_thread((thread_t*) list_next(task->threads));
	destroy_list(task->threads);
	destroy_directory(task->page_directory);
	kfree(task);
}

void task_add_thread(task_t* task, thread_t* thread) {
	list_push_back(task->threads, (uint32_t*) thread);
}
