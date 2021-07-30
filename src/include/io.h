#ifndef IO_H
#define IO_H
#include <stdlib.h>
char* read_file_contents(char* file);

char* create_buffer(size_t size);

void append_to_buffer(char* string);

void write_file(char* file, char* buffer);
#endif