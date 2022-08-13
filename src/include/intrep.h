#ifndef INTREP_H
#define INTREP_H

// Intermediate representation

#include "assembly.h"
#include <stdint.h>
#include "scope.h"
#include "wrapper.h"

#define INTREP_CHECKSTR ((char*) "INTREP")
#define INTREP_CHECKSTRLEN (sizeof(INTREP_CHECKSTR) - 1)

typedef union {
    uint64_t version; // 64
    struct {
        uint32_t major; // 32
        uint32_t minor; // 32
    };
} version_t;

version_t intrep_readversion(char* path);
version_t intrep_readversionbuf(char* buffer, size_t size);
void intrep_read(as_file_T* as_file, global_T* global, wrapper_t in);

void intrep_read_exe(as_file_T* as_file, wrapper_t in);
void intrep_read_lib(as_file_T* as_file, global_T* global, wrapper_t in);
void intrep_write_exe(as_file_T* as_file, wrapper_t out);
void intrep_write_lib(as_file_T* as_file, global_T* global, wrapper_t out);

#endif