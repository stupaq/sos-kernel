#ifndef COMMON_H
#define COMMON_H

#define NULL ((void*)0)

typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned char uint8_t;
typedef char int8_t;

#include <kprintf.h>
#include <panic.h>
#include <heap.h>
#include <string.h>

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

#endif // COMMON_H
