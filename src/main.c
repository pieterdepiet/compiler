#include "include/main.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "include/io.h"
#include "include/parser.h"
#include "include/args.h"
#include "include/visitor.h"
#include "include/assembly.h"
void call_assembler(char* as_path);
int compile(int argc, char* argv[]);
int main(int argc, char* argv[]) {
    return compile(argc, argv);
}
int compile(int argc, char* argv[]) {
    arg_options_T* arg_options = args_parse(argc, argv);
    
    char* file_contents = read_file_contents(arg_options->input_files[0]); // Read the contents of the file with name of the second arg
    lexer_T* lexer = init_lexer(file_contents); // Lexer: converts text into tokens
    parser_T* parser = init_parser(lexer); // Parser: converts tokens into AST (abstract syntax tree)
    printf(
        "+--------------+\n");
    AST_T* root = parser_parse(parser); // Get the root ast node
    printf(
        "|    Parsed    |\n"
        "+--------------+\n");
    as_file_T* as = init_as_file(); // As_file: stores global data and function definitions
    visitor_T* visitor = init_visitor(as); // Visitor: visits AST, makes a list of assembly operations and does things like type checking
    visitor_visit_global(visitor->global_scope, root); // Visit root ast
    printf(
        "|   Visited    |\n"
        "+--------------+\n");
    as_text_T* buf = as_compile_file(as); // Compile as_file to assembly
    printf(
        "| Assemblified |\n"
        "+--------------+\n"
    );
    char* assembly_file_location = "./program.s";
    write_file("./program.s", buf->buf);
    call_assembler(assembly_file_location);

    return 0;
}
#include "sys/wait.h"

void call_assembler(char* as_path) {
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        execl("/usr/bin/clang", "clang", "-c", "-o", "./program.o", as_path, NULL);
        exit(0);
    } else {
        /* command has executed */
        waitpid(-1, NULL, 0);
        printf(
            "|  Assembled   |\n"
            "+--------------+\n");
        pid = fork();
        if (pid==0) {
            execl("/usr/bin/ld", "ld", "./program.o", "./stdlib/lib.o", "-o", "./a.out", "-lSystem", NULL);
            exit(0);
        } else {
            printf(
                "|    Linked    |\n"
                "+--------------+\n");
            waitpid(-1, NULL, 0);
            return;
        }
    }
}