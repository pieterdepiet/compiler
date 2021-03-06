#include "include/utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int utils_strcmp(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0 && strlen(str1) == strlen(str2);
}
void utils_strcat(char** dest, const char* src) {
    *dest = realloc(*dest, (strlen(*dest) + strlen(src) + 1) * sizeof(char));
    strcat(*dest, src);
}

void list_add(void* list, size_t* size, void* element) {
    *size += 1;
    *((void**)list) = realloc(*((void**) list), *size * sizeof(void*));
    (*(void***)list)[*size-1] = element;   
}