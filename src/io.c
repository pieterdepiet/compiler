#include "include/io.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

char* read_file_contents(char* file) {
    FILE* fp = fopen(file, "rb");
    if (fp) {
        char* buffer;
        size_t len;
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = calloc(1 + len, sizeof(char));
        fread(buffer, 1, len, fp);
        fclose(fp);
        return buffer;
    } else {
        printf("Error while opening file: %s\n", strerror(errno));
        fclose(fp);
        exit(2);
        return (void*) 0;
    }
}
void write_file(char* file, char* buffer) {
    FILE* fp = fopen(file, "wb");
    if (fp) {
        fwrite(buffer, strlen(buffer), 1, fp);
    } else {
        printf("Error while opening file: %s\n", strerror(errno));
        exit(2);
    }
    fclose(fp);
}