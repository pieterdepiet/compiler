#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

char* reads() {
    char* buf = calloc(1024, sizeof(char));
    char* cur = buf;
    char c;
    while ((c = getchar()) != '\n') {
        *(cur++) = c;
    }
    *cur = '\0';
    buf = realloc(buf, strlen(buf) + 1);
    return buf;
}
char* readsmsg(char* msg) {
    printf("%s", msg);
    fflush(stdout);
    return reads();
}