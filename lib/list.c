#include <list.h>

list_t* list_new() {
	list_t* list = kmalloc(sizeof(list_t));
	list->tail = list->head = list->prev = list_entry_new();
	return list;
}

list_entry_t* list_entry_new() {
	return kmalloc_zero(sizeof(list_entry_t));
}

void destroy_list(list_t* list) {
	list_rewind(list);
	while (!list_empty(list))
		list_remove(list);
	// remove guardian
	kfree(list->head);
	kfree(list);
}

void destroy_list_entry(list_entry_t* entry) {
	kfree(entry);
}

uint32_t* list_current(list_t* list) {
	if (list->prev->next)
		return list->prev->next->entry;
	return 0;
}

uint32_t* list_next(list_t* list) {
	if (list->prev->next) {
		uint32_t* ptr = list->prev->next->entry;
		list->prev = list->prev->next;
		return ptr;
	}
	return 0;
}

void list_rewind(list_t* list) {
	list->prev = list->head;
}

uint32_t list_is_end(list_t* list) {
	return list->prev == list->tail;
}

static void __add_after(list_t* list, list_entry_t* after, list_entry_t* entry) {
	if (list->tail == after)
		list->tail = entry;
	entry->next = after->next;
	after->next = entry;
}

void list_push_back(list_t* list, uint32_t* ptr) {
	__add_after(list, list->tail, kmalloc(sizeof(list_entry_t)));
	list->tail->entry = ptr;
}

void list_remove(list_t* list) {
	list_entry_t* entry = list->prev->next;
	if (entry) {
		if (list->tail == entry) {
			if (entry->next)
				list->tail = entry->next;
			else
				list->tail = list->prev;
		}
		list->prev->next = entry->next;
		destroy_list_entry(entry);
	}
}

uint8_t list_empty(list_t* list) {
	return (list->tail == list->head);
}
