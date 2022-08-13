#include "include/invocation.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "include/errors.h"
#include "include/parser.h"
#include "include/visitor.h"
#include "include/intrep.h"
#include "include/assembly.h"
#include "include/previsit.h"
#include "include/utils.h"

int invocation_toobj(char* buf, size_t size, char* asname, char* objname) {
    FILE* f = fopen(asname, "w");
    fwrite(buf, sizeof(char), size, f);
    fclose(f);
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        execl("/usr/bin/as", "as", "-c", "-o", objname, asname, NULL);
        exit(0);
    } else {
        /* command has executed */
        waitpid(-1, NULL, 0);
        return 0;
    }
}

int invocation_visitfile(char* filename, global_T* global, as_file_T* as_file) {
    FILE* f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = calloc(1, size);
    fread(buf, sizeof(char), size, f);
    if (memcmp(buf, INTREP_CHECKSTR, INTREP_CHECKSTRLEN) == 0) {
        free(buf);
        fseek(f, 0, SEEK_SET);
        wrapper_t wrapper = wrapper_wrapf(f);
        intrep_read(as_file, global, wrapper);
    } else {
        lexer_T* lexer = init_lexer(buf); // Lexer: converts text into tokens
        lexer->filename = filename;
        parser_T* parser = init_parser(lexer); // Parser: converts tokens into AST (abstract syntax tree)
        printf(
            "+--------------+\n");
        AST_T* root = parser_parse(parser); // Get the root ast node
        printf(
            "|    Parsed    |\n"
            "+--------------+\n");
        root = previsit(root);
        printf(
            "|  Previsited  |\n"
            "+--------------+\n");
        visitor_T* visitor = init_visitor(as_file, global); // Visitor: visits AST, makes a list of assembly operations and does things like type checking
        visitor_visit_global(visitor->global_scope, root); // Visit root ast
        printf(
            "|   Visited    |\n"
            "+--------------+\n");
    }
    fclose(f);
    return 0;
}

int invocation_entry(int argc, char** argv) {
    enum {
        MODE_NONE,
        MODE_EXEC,
        MODE_OBJ,
        MODE_INTREP,
        MODE_LIST,
        MODE_LIB
    } mode = MODE_NONE;
    char** input_files = (void*) 0;
    size_t input_files_size = 0;
    char* outfile = (void*) 0;
    for (size_t i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') {
                if (argv[i][2] == '\0') {
                    input_files = realloc(input_files, ++input_files_size * sizeof(char*));
                    input_files[input_files_size-1] = argv[i];
                } else {
                    char* arg = &argv[i][2];
                    if (utils_strcmp(arg, "ix")) {
                        mode = MODE_INTREP;
                    } else if (utils_strcmp(arg, "il")) {
                        mode = MODE_LIB;
                    } else if (utils_strcmp(arg, "ls")) {
                        if (mode == MODE_NONE) {
                            mode = MODE_LIST;
                        } else {
                            fprintf(stderr, "multiple modes specified\n"); exit(EXIT_FAILURE);
                        }
                    } else {
                        err_unknown_option(argv[i]);
                    }
                }
            } else {
                switch (argv[i][1]) {
                    case 'o': if (outfile==(void*)0) {outfile = argv[++i];} else {fprintf(stderr, "multiple outputs specified\n"); exit(EXIT_FAILURE);} break;
                    case 'c': if (mode == MODE_NONE) {mode = MODE_OBJ;} else {fprintf(stderr, "multiple modes specified\n"); exit(EXIT_FAILURE);} break;
                    case 'x': if (mode == MODE_NONE) {mode = MODE_EXEC;} else {fprintf(stderr, "multiple modes specified\n"); exit(EXIT_FAILURE);} break;
                    case 'i': if (mode == MODE_NONE) {mode = MODE_INTREP;} else {fprintf(stderr, "multiple modes specified\n"); exit(EXIT_FAILURE);} break;
                    case 'l': {
                        // if (libname == (void*)0) {libname = argv[++i]; mode = MODE_LIB;} else {fprintf(stderr, "multiple library names specified\n"); exit(EXIT_FAILURE);} break;
                    } break;
                    default: {
                        input_files = realloc(input_files, ++input_files_size * sizeof(char*));
                        input_files[input_files_size-1] = argv[i];
                    } break;
                }
            }
        } else {
            input_files = realloc(input_files, ++input_files_size * sizeof(char*));
            input_files[input_files_size-1] = argv[i];
        }
    }
    if (mode == MODE_LIST) {
        for (size_t i = 0; i < input_files_size; i++) {
            version_t version = intrep_readversion(input_files[i]);
            if (version.version > 0) {
                printf("%s %u.%u\n", input_files[i], version.major, version.minor);
            } else {
                printf("Error\n");
            }
        }
    } else {
        as_file_T* as_file = init_as_file();
        global_T* global = init_global_scope(NULL);

        invocation_visitfile("./stdlib/headers", global, as_file);

        for (size_t i = 0; i < input_files_size; i++) {
            invocation_visitfile(input_files[i], global, as_file);
        }
        if (mode == MODE_NONE || mode == MODE_EXEC) {
            if (outfile == (void*) 0) {
                outfile = "./a.out";
            }
            char* assembly = as_compile_file(as_file);
            printf(
                "| Assemblified |\n"
                "+--------------+\n"
            );
            FILE* fp = fopen("program.s", "w");
            fwrite(assembly, sizeof(char), strlen(assembly), fp);
            fclose(fp);
            char* asname = "program.s";
            char* objname = "program.o";
            pid_t pid;
            pid = fork();
            if (pid == 0) {    
                execlp("as", "as", "-c", asname, "-o", objname, NULL);
                exit(0);
            } else {
                waitpid(-1, NULL, 0);
                printf(
                    "|  Assembled   |\n"
                    "+--------------+\n");
                pid = fork();
                if (pid == 0) {
                    execlp("ld", "ld", "-lSystem", objname, "./stdlib/lib.o", "-o", outfile, NULL);
                    exit(0);
                } else {
                    waitpid(-1, NULL, 0);
                    printf(
                        "|    Linked    |\n"
                        "+--------------+\n");
                    return 0;
                }
            }
        } else if (mode == MODE_INTREP) {
            if (outfile == (void*) 0) {
                outfile = "./rep.intrep";
            }
            FILE* out = fopen(outfile, "w");
            if (out) {
                wrapper_t wrapper = wrapper_wrapf(out);
                intrep_write_exe(as_file, wrapper);
            }
        } else if (mode == MODE_LIB) {
            if (outfile == (void*) 0) {
                outfile = "./lib.intrep";
            }
            FILE* out = fopen(outfile, "w");
            if (out) {
                wrapper_t wrapper = wrapper_wrapf(out);
                intrep_write_lib(as_file, global, wrapper);
            }
        }
    }
    return 1;
}