#include "include/import.h"
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
void import_import_package(scope_T* scope, char* package_name) {
    char* package_path;
    // if (strcmp(package_name - sizeof(".mycpkg"), ".mycpkg")==0) {
    package_path = package_name;
    // } else {
    //     package_path = calloc(1, (strlen(package_name) + sizeof(".mycpkg") + 1) * sizeof(char));
    //     strcpy(package_path, package_name);
    //     strcat(package_path, ".mycpkg");
    // }
    int command_fd[2];
    int err_fd[2];
    pipe(command_fd);
    pipe(err_fd);
    int pid = fork();

    if (pid == 0) { // Is child process
        dup2(command_fd[1], STDOUT_FILENO);
        dup2(err_fd[1], STDERR_FILENO);
        execl("/usr/bin/nm", "nm", "-j", package_path, NULL);
        close(err_fd[1]);
        close(command_fd[1]); // Write end
    } else {
        waitpid(-1, NULL, 0);
        struct stat s;
        struct stat err_stat;
        fstat(err_fd[0], &err_stat);
        if (err_stat.st_size > 0) {
            printf("Errno %d\n", errno);
        }
        fstat(command_fd[0], &s);
        char* buf = calloc(1, s.st_size * sizeof(char));
        read(command_fd[0], buf, s.st_size);
        printf("Command returned %s\n", buf);
        fstat(err_fd[0], &s);
        buf = realloc(buf, s.st_size * sizeof(char));
        read(err_fd[0], buf, s.st_size);
        printf("Stderr was %s\n", buf);
    }
    // fseek(program_fp, 0, SEEK_END);
    // size_t file_length = ftell(program_fp);
    // fseek(program_fp, 0, SEEK_SET);
    // char* buf = calloc(1, (file_length) * sizeof(char));
    // fread(buf, sizeof(char), file_length, program_fp);
    // size_t i = 0;
    
}