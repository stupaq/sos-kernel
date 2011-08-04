#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <sched/thread.h>
#include <common.h>

void init_scheduler(thread_t *initial_thread);

void thread_is_ready(thread_t *t);

void thread_not_ready(thread_t *t);

void schedule();

#endif
