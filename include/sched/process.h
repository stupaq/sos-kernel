#ifndef PROCESS_H
#define PROCESS_H

#include <common.h>
#include <sched/thread.h>
#include <mm/vmm.h>

#define PROCESS_RUNNING	1
#define PROCESS_WAITING	2
#define PROCESS_DYING	4

struct process {
	page_directory_t* page_directory; // aka page_tables_physical
	uint32_t* page_tables_virtual;
	uint32_t page_directory_physical;
	// better leave above variables as they are (process.s relies on that)
	uint32_t pid;
	thread_t* threads;
	struct process* next;
}__attribute__((packed));
typedef struct process process_t;

#endif // PROCESS_H
