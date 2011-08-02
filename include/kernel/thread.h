#ifndef THREAD_H
#define THREAD_H

#include <common.h>

struct thread {
	uint32_t esp, ebp, ebx, esi, edi, eflags;
	uint32_t id;
	struct thread* next; // at the end (explanation in thread.s)
};
typedef struct thread thread_t;


struct thread_list {
	thread_t* thread;
	struct thread_list* next;
};
typedef struct thread_list thread_list_t;

thread_t* init_threading();

thread_t* create_thread(int(*fn)(void*), void* arg, uint32_t* stack);

void switch_thread(thread_t* next);

#endif
