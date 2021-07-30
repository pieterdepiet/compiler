#include "include/main.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "include/io.h"
#include "include/parser.h"
#include "include/args.h"
// #include "include/preprocessor.h"
#include "include/visitor.h"
#include "include/assembly.h"
#include "include/import.h"
void call_assembler(char* as_path);
int compile(int argc, char* argv[]);
int main(int argc, char* argv[]) {
    return compile(argc, argv);
}
int compile(int argc, char* argv[]) {
    arg_options_T* arg_options = args_parse(argc, argv);
    
    char* file_contents = read_file_contents(arg_options->input_files[0]); // Read the contents of the file with name of the second arg
    // printf("%s", file_contents);
    lexer_T* lexer = init_lexer(file_contents); // Lexer: converts text into tokens
    parser_T* parser = init_parser(lexer); // Parser: converts tokens into AST (abstract syntax tree)
    AST_T* root = parser_parse(parser); // Get the root ast node
    // printf("Parsed\e[0m\n");
    as_file_T* as = init_as_file();
    visitor_T* visitor = init_visitor(as);
    visitor_visit_global(visitor, root);
    // printf("Visited\e[0m\n");
    // as_p_compile_global(as, root);
    as_text_T* buf = as_compile_file(as);
    // printf("Compiled\e[0m\n");
    char temp_file_name[L_tmpnam] = P_tmpdir "mycompiler-XXXXXX.s";
    // mkstemps(temp_file_name, 2);
    strcpy(temp_file_name, "./program.s");
    write_file(temp_file_name, buf->buf);
    call_assembler(temp_file_name);

    return 0;
}
#include "sys/wait.h"

void call_assembler(char* as_path) {
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        printf("Objectcodeify\n");
        execl("/usr/bin/clang", "clang", "-c", "-o", "./program.o", as_path, NULL);
        exit(0);
    } else {
        /* command has executed */
        waitpid(-1, NULL, 0);
        printf("Assembled\n");
        pid = fork();
        if (pid==0) {
            execl("/usr/bin/ld", "ld", "./program.o", "./stdlib/lib.o", "-o", "./a.out", "-lSystem", NULL);
            exit(0);
        } else {
            waitpid(-1, NULL, 0);
            printf("Linked\n");
            return;
        }
    }
}