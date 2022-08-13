#ifndef WRAPPER_H
#define WRAPPER_H
#include <sys/types.h>
#include <stdio.h>

extern int * __wrapper_error(void);
#define wrapper_errno (*__wrapper_error())

typedef struct WRAPPER *wrapper_t;
typedef size_t(*wrapper_readfn)(void*, size_t, void*);
typedef size_t(*wrapper_writefn)(void*, size_t, void*);
typedef off_t(*wrapper_seekfn)(off_t, int, void*);
typedef int(*wrapper_closefn)(void*);

wrapper_t wrapper_wrapf(FILE*);
wrapper_t wrapper_wrapbuf(char**, size_t*);

int wrapper_read(void*, size_t, wrapper_t);
int wrapper_write(void*, size_t, wrapper_t);
int wrapper_seek(off_t, int, wrapper_t);


#endif