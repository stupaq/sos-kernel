#ifndef COMMON_H
#define COMMON_H

#define NULL ((void*)0)

typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
typedef char int8_t;

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

#ifndef COMMON_H_NO_EXTRA_LIBS

#include <kernel/panic.h>
#include <kernel/keyboard.h>
#include <mm/heap.h>
#include <kprintf.h>
#include <string.h>

#endif // COMMON_H_NO_EXTRA_LIBS

#endif // COMMON_H
