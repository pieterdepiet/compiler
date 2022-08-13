#include "include/intrep_internal.h"
#include <stdlib.h>
#include <string.h>



void intrep_write_exe(as_file_T* as, wrapper_t wrapper) {
    wrapper_write(INTREP_CHECKSTR, INTREP_CHECKSTRLEN, wrapper);
    do {
        version_t version; // 0.1
        version.major = 0;
        version.minor = 1;
        wrapper_write(&version, sizeof(version), wrapper);
    } while (0);
    writei(wrapper, INTREP_EXE);
    flags_t flags = {0};
    off_t flagspos = wrapper_seek(sizeof(flags_t), SEEK_CUR, wrapper) - sizeof(flags_t);
    writesz(wrapper, as->data_size);
    for (size_t i = 0; i < as->data_size; i++) {
        as_data_T* data = as->data[i];
        intrep_write_as_data(wrapper, data, &flags);
    }
    writesz(wrapper, as->functions_size);
    for (size_t i = 0; i < as->functions_size; i++) {
        as_function_T* f = as->functions[i];
        intrep_write_as_function(wrapper, f, &flags);
    }
    writesz(wrapper, as->unnamed_strings_size);
    for (size_t i = 0; i < as->unnamed_strings_size; i++) {
        write_string(wrapper, as->unnamed_strings[i]);
    }
    off_t cur = wrapper_seek(0, SEEK_CUR, wrapper);
    wrapper_seek(flagspos, SEEK_SET, wrapper);
    wrapper_write(&flags, sizeof(flags_t), wrapper);
    wrapper_seek(cur, SEEK_SET, wrapper);
}
void intrep_read_exe(as_file_T* as, wrapper_t wrapper) {
    do {
        char check[INTREP_CHECKSTRLEN];
        wrapper_read(check, INTREP_CHECKSTRLEN, wrapper);
        if (memcmp(check, INTREP_CHECKSTR, INTREP_CHECKSTRLEN) != 0) {
            return;
        }
    } while (0);
    version_t version;
    wrapper_read(&version, sizeof(version), wrapper);
    int mode = readi(wrapper);
    if (mode != INTREP_EXE) {
        return;
    }
    flags_t flags;
    wrapper_read(&flags, sizeof(flags_t), wrapper);
    do {
        sz_t data_size = readsz(wrapper);
        as->data_size += data_size;
        as->data = realloc(as->data, as->data_size * sizeof(as_data_T*));
        for (size_t i = as->data_size - data_size; i < as->data_size; i++) {
            as->data[i] = intrep_read_as_data(wrapper);
        }
    } while (0);
    do {
        sz_t functions_size = readsz(wrapper);
        as->functions_size += functions_size;
        as->functions = calloc(1, as->functions_size * sizeof(as_function_T*));
        for (size_t j = as->functions_size - functions_size; j < as->functions_size; j++) {
            as->functions[j] = intrep_read_as_function(wrapper);
        }
    } while (0);
    do {
        sz_t strings_base = as->unnamed_strings_size;
        as->unnamed_strings = realloc(as->unnamed_strings, (as->unnamed_strings_size += readsz(wrapper)) * sizeof(char*));
        for (size_t i = strings_base; i < as->unnamed_strings_size; i++) {
            as->unnamed_strings[i] = read_string(wrapper);
        }
    } while (0);
}