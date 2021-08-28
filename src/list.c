#include "include/list.h"
#include <stdlib.h>
list_T* init_list() {
    list_T* list = calloc(1, sizeof(struct LIST_STRUCT));
    list->el = (void*) 0;
    list->size = 0;
    return list;
}

void list_add_list(list_T* list, void* element) {
    list->size++;
    list->el = realloc(list->el, list->size * sizeof(void*));
    list->el[list->size-1] = element;
}
void list_add(void* list, size_t* size, void* element) {
    *size += 1;
    *((void**)list) = realloc(*((void**) list), *size * sizeof(void*));
    (*(void***)list)[*size-1] = element;   
}