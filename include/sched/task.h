#ifndef TASK_H
#define TASK_H

#include <common.h>
#include <list.h>
#include <sched/thread.h>
#include <mm/vmm.h>

#define TASK_RUNNING	1
#define TASK_WAITING	2
#define TASK_DYING		4

struct task {
	page_directory_t* page_directory;
	// better leave above variables as they are (process.s would relie on that)
	uint32_t pid;
	list_t* threads;
};
typedef struct task task_t;

task_t* init_tasking(thread_t* init_thread);

task_t* create_task(thread_t* main_thread, page_directory_t* page_directory);

void add_thread(task_t* task, thread_t* thread);

void switch_task(task_t* next);

#endif // TASK_H
