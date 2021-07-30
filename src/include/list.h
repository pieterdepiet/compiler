#ifndef LIST_H
#define LIST_H
#include <stdlib.h>

typedef struct LIST_STRUCT {
    void** el;
    size_t size;
} list_T;

list_T* init_list();

void list_add(list_T* list, void* element);

#endif