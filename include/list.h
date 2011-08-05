#ifndef COLLECTIONS_H
#define COLLECTIONS_H

#include <common.h>

typedef struct list_entry {
	uint32_t* entry;
	struct list_entry* next;
} list_entry_t;

typedef struct list {
	list_entry_t* head, *tail;
	list_entry_t* prev;
} list_t;


list_t* list_new();

list_entry_t* list_entry_new();

void destroy_list(list_t* list);

void destroy_list_entry(list_entry_t* entry);

uint8_t list_empty(list_t* list);

uint32_t* list_current(list_t* list);

uint32_t* list_next(list_t* list);

uint32_t list_is_end(list_t* list);

void list_rewind(list_t* list);

void list_push_back(list_t* list, uint32_t* ptr);

void list_remove(list_t* list);

#endif // COLLECTIONS_H
