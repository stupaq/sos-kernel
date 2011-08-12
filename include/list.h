#ifndef COLLECTIONS_H
#define COLLECTIONS_H

#include <common.h>

typedef struct list {
	uint32_t** elems;
	uint32_t capacity, size, curr;
} list_t;

list_t* list_new();

void destroy_list(list_t* list);

uint8_t list_is_empty(list_t* list);

uint32_t list_is_end(list_t* list);

uint32_t* list_current(list_t* list);

uint32_t* list_next(list_t* list);

uint32_t* list_cyc_next(list_t* list);

void list_rewind(list_t* list);

void list_push_back(list_t* list, uint32_t* ptr);

void list_remove(list_t* list);

#endif // COLLECTIONS_H
