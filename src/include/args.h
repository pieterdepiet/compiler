#ifndef ARGS_H
#define ARGS_H
#include <stdlib.h>

typedef struct ARG_OPTIONS_STRUCT {
    char* outfile;
    char** input_files;
    size_t input_files_size;
} arg_options_T;

arg_options_T* args_parse(int argc, char* argv[]);

#endif