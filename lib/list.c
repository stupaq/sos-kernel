#include <list.h>

#define INITIAL_CAPACITY 8

inline static uint32_t __umin(uint32_t a, uint32_t b) {
	return (a < b) ? a : b;
}

inline static void __resize(list_t* list, uint32_t desired_capacity) {
	if (INITIAL_CAPACITY > desired_capacity)
		return;
	// compute new capacity
	uint32_t new_capacity = list->capacity + (list->capacity == 0);
	while (desired_capacity > new_capacity)
		new_capacity *= 2;
	while (2 * desired_capacity < new_capacity)
		new_capacity /= 2;
	// allocate place for new data
	uint32_t** new_elems = (uint32_t**) kmalloc_zero(
			sizeof(uint32_t*) * new_capacity);
	if (list->elems) {
		// copy data
		memcpy(new_elems, list->elems, __umin(list->capacity, new_capacity));
		// free old data
		kfree(list->elems);
	}
	// setup list
	list->capacity = new_capacity;
	list->elems = new_elems;
}

list_t* list_new() {
	list_t* list = kmalloc_zero(sizeof(list_t));
	list->capacity = INITIAL_CAPACITY;
	list->elems = kmalloc(sizeof(uint32_t*) * 6);
	// TODO: debug __resize()
	//__resize(list, INITIAL_CAPACITY);
	return list;
}

void destroy_list(list_t* list) {
	kfree(list->elems);
	kfree(list);
}

uint32_t* list_current(list_t* list) {
	if (list->curr < list->size)
		return list->elems[list->curr];
	return 0;
}

uint32_t* list_next(list_t* list) {
	list->curr++;
	return list_current(list);
}

uint32_t* list_cyc_next(list_t* list) {
	if (list_is_end(list))
		list_rewind(list);
	return list_next(list);
}

void list_rewind(list_t* list) {
	list->curr = 0;
}

void list_push_back(list_t* list, uint32_t* ptr) {
	//__resize(list, list->size + 1);
	list->elems[list->size++] = ptr;
}

void list_remove(list_t* list) {
	if (list_is_end(list))
		return;
	// change current with last
	uint32_t* tmp = list->elems[list->size - 1];
	list->elems[list->size - 1] = list->elems[list->curr];
	list->elems[list->curr] = tmp;
	// remove last
	list->size--;
	// update capacity
	//__resize(list, list->size);
}

uint8_t list_is_empty(list_t* list) {
	return (list->size == 0);
}

uint32_t list_is_end(list_t* list) {
	return list->curr >= list->size;
}
