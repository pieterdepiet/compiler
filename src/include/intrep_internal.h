#ifndef INTREP_INTERNAL_H
#define INTREP_INTERNAL_H
#include "intrep.h"
#include "wrapper.h"

typedef uint32_t sz_t;

sz_t readsz(wrapper_t wrapper);
int32_t readi(wrapper_t wrapper);
char* read_string(wrapper_t wrapper);
as_function_T* intrep_read_as_function(wrapper_t wrapper);

enum intrep_type {
    INTREP_EXE,
    INTREP_LIB
};

void writesz(wrapper_t wrapper, sz_t val);
void writei(wrapper_t wrapper, int32_t val);
void write_string(wrapper_t wrapper, char* val);
void intrep_write_as_function(wrapper_t wrapper, as_function_T* f, flags_t* flags);
void intrep_write_as_data(wrapper_t wrapper, as_data_T* data, flags_t* flags);

#endif