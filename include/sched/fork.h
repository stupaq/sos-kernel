#ifndef FORK_H
#define FORK_H

#include <common.h>

int32_t fork_userspace();

uint32_t exec_elf(const char* name);

uint32_t exec_thread(int(*fn)(void*), void* arg);

#endif // FORK_H
