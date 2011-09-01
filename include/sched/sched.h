#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <common.h>
#include <sched/thread.h>
#include <sched/task.h>

void init_scheduler(task_t* initial_task);

void schedule_add_task(task_t* new_task);

void preempt();

void sleep();

void wakeup(thread_t* thread);

void kill(thread_t* thread);

#endif
