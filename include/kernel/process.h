#ifndef PROCESS_H
#define PROCESS_H

#include <common.h>
#include <kernel/thread.h>
#include <kernel/scheduler.h>
#include <mm/vmm.h>

typedef struct process {
	struct process* next;
	uint32_t pid;
	page_directory_t page_directory;
	thread_list_t threads;
} process_t;

#endif // PROCESS_H