#ifndef INTREP_H
#define INTREP_H

// Intermediate representation

#include "assembly.h"
#include <stdint.h>

#define INTREP_CHECKSTR ((char*) "INTREP")
#define INTREP_CHECKSTRLEN (sizeof(INTREP_CHECKSTR) - 1)

typedef struct sized_buffer {
    size_t size;
    char* buffer;
} sbuf_T;
typedef union {
    uint64_t version; // 64
    struct {
        uint32_t major; // 32
        uint32_t minor; // 32
    };
} version_t;

as_file_T* intrep_read(char* path);
version_t intrep_readversion(char* path);
version_t intrep_readversionbuf(char* buffer, size_t size);
int intrep_write(as_file_T* as_file, char* path);

as_file_T* intrep_readbuf0_1(char* buffer, size_t size, as_file_T* as_file);

sbuf_T intrep_filetobuf(as_file_T* as_file);

#endif