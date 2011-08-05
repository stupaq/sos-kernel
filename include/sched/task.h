#ifndef PROCESS_H
#define PROCESS_H

#include <common.h>
#include <sched/thread.h>
#include <mm/vmm.h>

#define TASK_RUNNING	1
#define TASK_WAITING	2
#define TASK_DYING		4

struct task {
	page_directory_t* page_directory;
	// better leave above variables as they are (process.s would relie on that)
	uint32_t pid;
	thread_t* threads;
	struct task* next;
}__attribute__((packed));
typedef struct task task_t;

#endif // PROCESS_H
