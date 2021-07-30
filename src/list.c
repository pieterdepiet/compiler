#include "include/list.h"
#include "stdio.h"
list_T* init_list() {
    list_T* list = calloc(1, sizeof(struct LIST_STRUCT));
    list->el = (void*) 0;
    list->size = 0;
    return list;
}

void list_add(list_T* list, void* element) {
    list->size++;
    list->el = realloc(list->el, list->size * sizeof(void*));
    list->el[list->size-1] = element;
}