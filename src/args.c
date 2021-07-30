#include "include/args.h"
#include <stdlib.h>
#include "include/utils.h"
#include "include/errors.h"

arg_options_T* args_parse(int argc, char* argv[]) {
    if (argc < 2) {
        err_no_file_specified();
    }
    arg_options_T* args = calloc(1, sizeof(struct ARG_OPTIONS_STRUCT));
    for (size_t i = 1; i < argc; i++) {
        if (utils_strcmp(argv[i], "-o")) {
            i++;
            args->outfile = argv[i];
        } else if (argv[i][0] == '-') {
            err_unknown_option(argv[i]);
        } else {
            args->input_files_size++;
            args->input_files = realloc(args->input_files, args->input_files_size * sizeof(char*));
            args->input_files[args->input_files_size-1] = argv[i];
        }
    }
    return args;
}