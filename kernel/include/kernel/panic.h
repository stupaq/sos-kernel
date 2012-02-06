#ifndef PANIC_H
#define PANIC_H

// prints calls stack
void print_stack_trace();

// kernel panic
void panic(const char *msg);

#endif
