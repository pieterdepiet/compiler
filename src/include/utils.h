#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

int utils_strcmp(const char* str1, const char* str2);

void utils_strcat(char** dest, const char* src);

void list_add(void* list, size_t* size, void* element);

#endif