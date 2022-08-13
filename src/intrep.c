#include "include/intrep_internal.h"
#include "include/assembly.h"
#include "include/wrapper.h"
#include "include/errors.h"
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "include/utils.h"

version_t intrep_readversion(char* path) {
    FILE* fp = fopen(path, "r");
    char checkstr[INTREP_CHECKSTRLEN];
    version_t version;
    if (fread(checkstr, sizeof(char), INTREP_CHECKSTRLEN, fp) != INTREP_CHECKSTRLEN || memcmp(checkstr, INTREP_CHECKSTR, INTREP_CHECKSTRLEN) != 0 || fread(&version, sizeof(version_t), 1, fp) != 1) {
        version.major = 0;
        version.minor = 0;
        return version;
    }
    return version;
}
version_t intrep_readversionbuf(char* buf, size_t size) {
    version_t version;
    if (size < INTREP_CHECKSTRLEN + sizeof(version_t) || memcmp(buf, INTREP_CHECKSTR, INTREP_CHECKSTRLEN) != 0) {
        version.major = 0;
        version.minor = 0;
    }
    version = *((version_t*) &buf[INTREP_CHECKSTRLEN]);
    return version;
}
void intrep_read(as_file_T* as_file, global_T* global, wrapper_t in) {
    char checkstr[INTREP_CHECKSTRLEN];
    wrapper_read(checkstr, INTREP_CHECKSTRLEN, in);
    if (memcmp(checkstr, INTREP_CHECKSTR, INTREP_CHECKSTRLEN) == 0) {
        version_t version;
        wrapper_read(&version, sizeof(version), in);
        int mode = readi(in);
        wrapper_seek(0, SEEK_SET, in);
        if (mode == INTREP_LIB) {
            intrep_read_lib(as_file, global, in);
        } else if (mode == INTREP_EXE) {
            intrep_read_exe(as_file, in);
        }
    }
}

sz_t readsz(wrapper_t wrapper) {
    sz_t val;
    wrapper_read(&val, sizeof(val), wrapper);
    return val;
}
int32_t readi(wrapper_t wrapper) {
    int32_t val;
    wrapper_read(&val, sizeof(val), wrapper);
    return val;
}

char* read_string(wrapper_t wrapper) {
    sz_t len = readsz(wrapper);
    char* value = malloc(len + 1);
    wrapper_read(value, len, wrapper);
    value[len] = '\0';
    return value;
}

as_function_T* intrep_read_as_function(wrapper_t wrapper) {
    as_function_T* f = calloc(1, sizeof(as_function_T));
    f->name = read_string(wrapper);
    f->scope_size = readsz(wrapper);
    f->argc = readsz(wrapper);
    f->function_no = readsz(wrapper);
    size_t operations_size = (f->operations_size = readsz(wrapper));
    f->operations = calloc(1, operations_size * sizeof(as_op_T*));
    for (size_t i = 0; i < operations_size; i++) {
        as_op_T* op = (f->operations[i] = calloc(1, sizeof(as_op_T)));
        
        op->type = readi(wrapper);
        int32_t t = op->type;
        if (t == ASOP_SETLASTIMM || t == ASOP_BINOP) {
            op->data_type = readi(wrapper);
            if (t == ASOP_SETLASTIMM) {
                op->value.int_value = readi(wrapper);
            } else {
                op->binop_type = readi(wrapper);
                goto readopsize;
            }
        } else if (t == ASOP_VREF || t == ASOP_VMOD || t == ASOP_ARGTOSTACK) {
            op->var_location = readsz(wrapper);
            if (t == ASOP_ARGTOSTACK) {
                op->argno = readi(wrapper);
            } else if (t == ASOP_VREF) {
                goto readopsizeend;
            }
            goto readopsize;
        } else if (t == ASOP_FCALL || t == ASOP_SYMBADDRREF || t == ASOP_SYMBOLREF) {
            op->name = read_string(wrapper);
        } else if (t == ASOP_MEMBREF || t == ASOP_LOCALMEMB || t == ASOP_PTRMEMBMOD) {
            op->memb_offset = readsz(wrapper);
            if (t == ASOP_PTRMEMBMOD) {
                goto readopsize;
            }
        } else if (t == ASOP_JCOND || t == ASOP_JMP || t == ASOP_BB) {
            op->bb_no = readsz(wrapper);
        } else if (t == ASOP_STRINGREF) {
            op->string_index = readsz(wrapper);
        } else if (t == ASOP_SETLASTCMP) {
            op->cmp_type = readi(wrapper);
        } else if (t == ASOP_ARGTOREG) {
            op->argno = readi(wrapper);
            goto readopsize;
        } else if (t == ASOP_UNOP) {
            op->unop_type = readi(wrapper);
            goto readopsize;
        } else if (t == ASOP_RETURN || t == ASOP_NEXTREG || t == ASOP_NEW || t == ASOP_MEMTOREG || t == ASOP_PUSHREG || t == ASOP_LOCALMEMBMOD || t == ASOP_INDEXMOD) {
            goto readopsize;
        }

        goto readopsizeend;
        readopsize:
        op->op_size = readsz(wrapper);
        readopsizeend: continue;
    }
    return f;
}

void writesz(wrapper_t wrapper, sz_t val) {
    wrapper_write(&val, sizeof(sz_t), wrapper);
}
void writei(wrapper_t wrapper, int32_t val) {
    wrapper_write(&val, sizeof(int32_t), wrapper);
}
void write_string(wrapper_t wrapper, char* val) {
    sz_t len = strlen(val);
    writesz(wrapper, len);
    wrapper_write(val, len, wrapper);
}
void intrep_write_as_function(wrapper_t wrapper, as_function_T* f, flags_t* flags) {
    write_string(wrapper, f->name);
    writesz(wrapper, f->scope_size);
    writesz(wrapper, f->argc);
    writesz(wrapper, f->function_no);
    writesz(wrapper, f->operations_size);
    for (size_t i = 0; i < f->operations_size; i++) {
        as_op_T* op = f->operations[i];
        if (op->op_size > 4 && op->type != ASOP_NEW) {
            flags->needs_64 = 1;
        }
        writei(wrapper, op->type);
        int32_t t = op->type;
        if (t == ASOP_SETLASTIMM || t == ASOP_BINOP) {
            writei(wrapper, op->data_type);
            if (t == ASOP_SETLASTIMM) {
                writei(wrapper, op->value.int_value);
            } else {
                writei(wrapper, op->binop_type);
                goto writeopsize;
            }
        } else if (t == ASOP_VREF || t == ASOP_VMOD || t == ASOP_ARGTOSTACK) {
            writesz(wrapper, op->var_location);
            if (t == ASOP_ARGTOSTACK) {
                writei(wrapper, op->argno);
            } else if (t == ASOP_VREF) {
                goto writeopsizeend;
            }
            goto writeopsize;
        } else if (t == ASOP_FCALL || t == ASOP_SYMBADDRREF || t == ASOP_SYMBOLREF) {
            write_string(wrapper, op->name);
        } else if (t == ASOP_MEMBREF || t == ASOP_LOCALMEMB || t == ASOP_PTRMEMBMOD) {
            writesz(wrapper, op->memb_offset);
            if (t == ASOP_PTRMEMBMOD) {
                goto writeopsize;
            }
        } else if (t == ASOP_JCOND || t == ASOP_JMP || t == ASOP_BB) {
            writesz(wrapper, op->bb_no);
        } else if (t == ASOP_STRINGREF) {
            writesz(wrapper, op->string_index);
        } else if (t == ASOP_SETLASTCMP) {
            writei(wrapper, op->cmp_type);
        } else if (t == ASOP_ARGTOREG) {
            writei(wrapper, op->argno);
            goto writeopsize;
        } else if (t == ASOP_UNOP) {
            writei(wrapper, op->unop_type);
            goto writeopsize;
        } else if (t == ASOP_RETURN || t == ASOP_NEXTREG || t == ASOP_NEW || t == ASOP_MEMTOREG || t == ASOP_PUSHREG || t == ASOP_LOCALMEMBMOD || t == ASOP_INDEXMOD) {
            goto writeopsize;
        }

        goto writeopsizeend;
        writeopsize:
        writesz(wrapper, op->op_size);
        writeopsizeend: continue;
    }
}
void intrep_write_as_data(wrapper_t wrapper, as_data_T* data, flags_t* flags) {
    wrapper_write(&data->type, sizeof(int32_t), wrapper);
    write_string(wrapper, data->name);
    switch (data->type) {
        case ASTYPE_CHAR: {
            wrapper_write(&data->value, sizeof(int8_t), wrapper);
        } break;
        case ASTYPE_SHORT: {
            wrapper_write(&data->value, sizeof(int16_t), wrapper);
        } break;
        case ASTYPE_INT: {
            wrapper_write(&data->value, sizeof(int32_t), wrapper);
        } break;
        case ASTYPE_LONG: {
            flags->needs_64 = 1;
            wrapper_write(&data->value, sizeof(int64_t), wrapper);
        } break;
        case ASTYPE_STRING: {
            write_string(wrapper, data->value.ptr_value);
        } break;
    }
}