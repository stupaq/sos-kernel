#ifndef STRING_H
#define STRING_H

#include <common.h>

void* memcpy(void* dest, const void* src, uint32_t len);

void* memset(void* dest, int val, uint32_t len);

int strcmp(const char* str1, const char* str2);

char* strcpy(char* dest, const char* src);

char* strcat(char* dest, const char* src);

int strlen(char* src);

#endif // STRING_H
