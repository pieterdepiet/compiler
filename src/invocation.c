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

int invocation_entry(int argc, char** argv) {
    enum {
        MODE_NONE,
        MODE_EXEC,
        MODE_OBJ,
        MODE_INTREP,
        MODE_LIST
    } mode = MODE_NONE;
    char** input_files = (void*) 0;
    size_t input_files_size = 0;
    char* outfile = (void*) 0;
    for (size_t i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') {
                input_files = realloc(input_files, ++input_files_size * sizeof(char*));
                input_files[input_files_size-1] = argv[i];
            } else {
                switch (argv[i][1]) {
                    case 'o': if (outfile==(void*)0) {outfile = argv[++i];} else {fprintf(stderr, "multiple outputs specified\n"); exit(EXIT_FAILURE);} break;
                    case 'c': if (mode == MODE_NONE) {mode = MODE_OBJ;} else {fprintf(stderr, "multiple modes specified\n"); exit(EXIT_FAILURE);} break;
                    case 'x': if (mode == MODE_NONE) {mode = MODE_EXEC;} else {fprintf(stderr, "multiple modes specified\n"); exit(EXIT_FAILURE);} break;
                    case 'i': if (mode == MODE_NONE) {mode = MODE_INTREP;} else {fprintf(stderr, "multiple modes specified\n"); exit(EXIT_FAILURE);} break;
                    case 'l': if (mode == MODE_NONE) {mode = MODE_LIST;} else {fprintf(stderr, "multiple modes specified\n"); exit(EXIT_FAILURE);} break;
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
        // as_file_T** as_files = (void*) 0;
        // size_t as_files_size = 0;
        as_file_T* as_file = init_as_file();
        global_T* scope = init_global_scope(NULL);
        for (size_t i = 0; i < input_files_size; i++) {
            FILE* f = fopen(input_files[i], "r");
            fseek(f, 0, SEEK_END);
            size_t size = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* buf = calloc(1, size);
            fread(buf, sizeof(char), size, f);
            // as_files = realloc(as_files, ++as_files_size * sizeof(as_file_T*));
            if (memcmp(buf, INTREP_CHECKSTR, INTREP_CHECKSTRLEN) == 0) {
                // as_files[as_files_size-1] = intrep_readbuf0_1(buf, size);
                intrep_readbuf0_1(buf, size, as_file);
            } else {
                lexer_T* lexer = init_lexer(buf); // Lexer: converts text into tokens
                parser_T* parser = init_parser(lexer); // Parser: converts tokens into AST (abstract syntax tree)
                printf(
                    "+--------------+\n");
                AST_T* root = parser_parse(parser); // Get the root ast node
                printf(
                    "|    Parsed    |\n"
                    "+--------------+\n");
                visitor_T* visitor = init_visitor(as_file, scope); // Visitor: visits AST, makes a list of assembly operations and does things like type checking
                visitor_visit_global(visitor->global_scope, root); // Visit root ast
                printf(
                    "|   Visited    |\n"
                    "+--------------+\n");
                // as_files[as_files_size-1] = as;
            }
            fclose(f);
        }
        // buf = realloc(buf, ++bufsize);
        // buf[bufsize - 1] = 0;
        // lexer_T* lexer = init_lexer(buf); // Lexer: converts text into tokens
        // parser_T* parser = init_parser(lexer); // Parser: converts tokens into AST (abstract syntax tree)
        // printf(
        //     "+--------------+\n");
        // AST_T* root = parser_parse(parser); // Get the root ast node
        // printf(
        //     "|    Parsed    |\n"
        //     "+--------------+\n");
        // as_file_T* as = init_as_file(); // As_file: stores global data and function definitions
        // visitor_T* visitor = init_visitor(as); // Visitor: visits AST, makes a list of assembly operations and does things like type checking
        // visitor_visit_global(visitor->global_scope, root); // Visit root ast
        // printf(
        //     "|   Visited    |\n"
        //     "+--------------+\n");
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
            // if (pid == 0) {
            //     execl("/usr/bin/clang", "clang", "-o", outfile, "./program.s", "./stdlib/lib.o", NULL);
            //     exit(0);
            // } else {
            //     /* command has executed */
            //     waitpid(-1, NULL, 0);
            //     printf(
            //         "|  Assembled   |\n"
            //         "+--------------+\n");
            //     // pid = fork();
            //     // if (pid==0) {
            //     //     execl("/usr/bin/ld", "ld", "./program.o", "./stdlib/lib.o", "-o", "./a.out", "-lSystem", NULL);
            //     //     exit(0);
            //     // } else {
            //         printf(
            //             "|    Linked    |\n"
            //             "+--------------+\n");
            //         waitpid(-1, NULL, 0);
            //         return 0;
            //     // }
            // }
        } else if (mode == MODE_INTREP) {
            if (outfile == (void*) 0) {
                outfile = "./rep.intrep";
            }
            intrep_write(as_file, outfile);
        }
    }
    return 1;
}