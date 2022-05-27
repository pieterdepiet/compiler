#include "include/intrep.h"
#include "include/assembly.h"
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

sbuf_T* sbuf_create(size_t size, char* buffer) {
    sbuf_T* sbuf = calloc(1, sizeof(sbuf_T));
    sbuf->size = size;
    sbuf->buffer = buffer;
    return sbuf;
}

as_file_T* intrep_read(char* path) {
    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        return (void*) 0;
    }
    size_t bufsize = 0;
    char* buffer = calloc(1, 1024*sizeof(char));
    size_t read;
    while ((read = fread(buffer, sizeof(char), 1024, fp)) > 0) {
        buffer = realloc(buffer, (bufsize += read) * sizeof(char));
    }
    fclose(fp);
    version_t version = intrep_readversionbuf(buffer, bufsize);
    if (version.major == 0 && version.minor == 1) {
        as_file_T* file = intrep_readbuf0_1(buffer, bufsize, init_as_file());
        free(buffer);
        return file;
    } else {
        return (void*) 0;
    }
    
}
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
int intrep_write(as_file_T* as_file, char* path) {
    sbuf_T sbuf = intrep_filetobuf(as_file);
    struct stat st;
    if (stat(path, &st) == -1) {
        if (errno == ENOENT) {
            int fd;
            if ((fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
                return -1;
            }
            close(fd);
        } else {
            return -1;
        }
    }

    FILE* fp = fopen(path, "w");

    if (fp == NULL) {
        return -1;
    }
    fwrite(sbuf.buffer, sizeof(char), sbuf.size, fp);
    fclose(fp);
    return 0;
}


typedef uint32_t sz_t; // 32

as_file_T* intrep_readbuf0_1(char* buf, size_t size, as_file_T* as) {
    if (memcmp(buf, INTREP_CHECKSTR, INTREP_CHECKSTRLEN) != 0) {
        return (void*) 0;
    }
    // as_file_T* as = init_as_file();
    char* cur = &buf[INTREP_CHECKSTRLEN];
    version_t version = *((version_t*) cur);
    if (version.major != 0 && version.minor != 1) {
        return (void*) 0;
    }
    sz_t libraries_size = *((sz_t*) (cur = &cur[sizeof(version_t)]));
    flags_t flags = *((flags_t*) (cur = &cur[sizeof(sz_t)]));
    as->flags = flags;
    do {
        sz_t data_size = *((sz_t*) (cur = &cur[sizeof(flags_t)]));
        as->data_size += data_size;
        as->data = realloc(as->data, as->data_size * sizeof(as_data_T*));
        cur = &cur[sizeof(sz_t)];
        for (size_t i = as->data_size - data_size; i < as->data_size; i++) {
            as_data_T* data = (as->data[i] = calloc(1, sizeof(as_data_T)));
            data->type = *((int32_t*) cur);
            do {
                sz_t namelen = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                data->name = calloc(1, (namelen + 1) * sizeof(char));
                memcpy(data->name, (cur = &cur[sizeof(sz_t)]), namelen);
                data->name[namelen] = '\0';
                cur = &cur[namelen];
            } while (0);
            switch (data->type) {
                case ASTYPE_CHAR: {
                    data->value.char_value = *((int8_t*) cur);
                    cur = &cur[1];
                } break;
                case ASTYPE_SHORT: {
                    data->value.short_value = *((int16_t*) cur);
                    cur = &cur[2];
                } break;
                case ASTYPE_INT: {
                    data->value.int_value = *((int32_t*) cur);
                    cur = &cur[4];
                } break;
                case ASTYPE_LONG: {
                    *((int64_t*) cur) = data->value.long_value;
                    cur = &cur[8];
                } break;
                case ASTYPE_STRING: {
                    sz_t len = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                    data->value.ptr_value = calloc(1, (len + 1) * sizeof(char));
                    memcpy(data->value.ptr_value, (cur = &cur[sizeof(sz_t)]), len);
                    ((char*) data->value.ptr_value)[len] = '\0';
                    cur = &cur[len];
                } break;
            }
        }
    } while (0);
    do {
        sz_t functions_size = *((sz_t*) (cur));
        as->functions_size += functions_size;
        as->functions = calloc(1, as->functions_size * sizeof(as_function_T*));
        cur = &cur[sizeof(sz_t)];
        for (size_t j = as->functions_size - functions_size; j < as->functions_size; j++) {
            as_function_T* f = (as->functions[j] = calloc(1, sizeof(as_function_T)));
            do {
                sz_t namelen = *((sz_t*) (cur));
                f->name = calloc(1, (namelen + 1) * sizeof(char));
                memcpy(f->name, (cur = &cur[sizeof(sz_t)]), namelen);
                f->name[namelen] = '\0';
                f->scope_size = *((sz_t*) (cur = &cur[namelen]));
                f->argc = *((sz_t*) (cur = &cur[sizeof(sz_t)]));
                f->function_no = *((sz_t*) (cur = &cur[sizeof(sz_t)]));
            } while (0);
            do {
                size_t operations_size = *((sz_t*) (cur = &cur[sizeof(sz_t)]));
                f->operations_size = operations_size;
                f->operations = calloc(1, operations_size * sizeof(as_op_T*));
                cur = &cur[sizeof(sz_t)];
                for (size_t i = 0; i < operations_size; i++) {
                    as_op_T* op = (f->operations[i] = calloc(1, sizeof(as_op_T)));
                    op->type = *((int32_t*) cur);
                    switch (op->type) {
                        case ASOP_NOP: case ASOP_FREEREG: case ASOP_POPREG: case ASOP_FREEPTRREG: case ASOP_PTRTOREG: {
                            cur = &cur[sizeof(int32_t)];
                        } break;
                        case ASOP_RETURN: case ASOP_NEXTREG:  case ASOP_NEW: case ASOP_MEMTOREG: case ASOP_PUSHREG: case ASOP_LOCALMEMBMOD: {
                            op->op_size = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            cur = &cur[sizeof(sz_t)];
                        } break;
                        case ASOP_ARGTOSTACK: {
                            op->op_size = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            op->argno = *((sz_t*) (cur = &cur[sizeof(sz_t)]));
                            op->var_location = *((sz_t*) (cur = &cur[sizeof(sz_t)]));
                            cur = &cur[sizeof(sz_t)];
                        } break;
                        case ASOP_ARGTOREG: {
                            op->op_size = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            op->argno = *((sz_t*) (cur = &cur[sizeof(sz_t)]));
                            cur = &cur[sizeof(sz_t)];
                        } break;
                        case ASOP_SETLASTIMM: {
                            op->value.int_value = *((int32_t*) (cur = &cur[sizeof(int32_t)]));
                            op->data_type = *((int32_t*) (cur = &cur[sizeof(int32_t)]));
                            cur = &cur[sizeof(int32_t)];
                        } break;
                        case ASOP_VMOD: {
                            op->op_size = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            op->var_location = *((sz_t*) (cur = &cur[sizeof(sz_t)]));
                            cur = &cur[sizeof(sz_t)];
                        } break;
                        case ASOP_VREF: {
                            op->var_location = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            cur = &cur[sizeof(sz_t)];
                        } break;
                        case ASOP_FCALL: case ASOP_SYMBADDRREF: case ASOP_SYMBOLREF: {
                            sz_t len = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            op->name = calloc(1, (len + 1) * sizeof(char));
                            memcpy(op->name, (cur = &cur[sizeof(sz_t)]), len);
                            op->name[len] = '\0';
                            cur += len;
                        } break;
                        case ASOP_RETVAL: case ASOP_RETNULL: case ASOP_LEA: {
                            cur = &cur[sizeof(int32_t)];
                        } break;
                        case ASOP_BINOP: {
                            op->op_size = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            op->binop_type = *((int32_t*) (cur = &cur[sizeof(sz_t)]));
                            op->data_type = *((int32_t*) (cur = &cur[sizeof(int32_t)]));
                            cur = &cur[sizeof(int32_t)];
                        } break;
                        case ASOP_MEMBREF: case ASOP_LOCALMEMB: {
                            op->memb_offset = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            cur = &cur[sizeof(sz_t)];
                        } break;
                        case ASOP_PTRMEMBMOD: {
                            op->op_size = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            op->memb_offset = *((sz_t*) (cur = &cur[sizeof(sz_t)]));
                            cur = &cur[sizeof(sz_t)];
                        } break;
                        case ASOP_JCOND: case ASOP_JMP: case ASOP_BB: {
                            op->bb_no = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            cur = &cur[sizeof(sz_t)];
                        } break;
                        case ASOP_UNOP: {
                            op->type = *((int32_t*) cur);
                            op->op_size = *((sz_t*) (cur = &cur[sizeof(int32_t)]));
                            op->unop_type = *((int32_t*) (cur = &cur[sizeof(sz_t)]));
                            cur = &cur[sizeof(int32_t)];
                        } break;
                        case ASOP_STRINGREF: {
                            op->type = *((int32_t*) cur);
                            op->string_index = *((sz_t*) (cur = &cur[sizeof(int32_t)])) + as->unnamed_strings_size; // add base
                            cur = &cur[sizeof(sz_t)];
                        } break;
                        case ASOP_SETLASTCMP: {
                            op->cmp_type = *((int32_t*) (cur = &cur[sizeof(int32_t)]));
                            cur = &cur[sizeof(int32_t)];
                        } break;
                    }
                }
            } while (0);
        }
    } while (0);
    do {
        sz_t strings_base = as->unnamed_strings_size;
        as->unnamed_strings = realloc(as->unnamed_strings, (as->unnamed_strings_size += *((sz_t*) cur)) * sizeof(char*));
        cur = &cur[sizeof(sz_t)];
        for (size_t i = strings_base; i < as->unnamed_strings_size; i++) {
            sz_t len = *((sz_t*) cur);
            as->unnamed_strings[i] = calloc(1, len + 1);
            memcpy(as->unnamed_strings[i], (cur = &cur[sizeof(sz_t)]), len);
            as->unnamed_strings[i][len] = '\0';
            cur = &cur[len];
        }
    } while (0);
    return as;
}


sbuf_T intrep_filetobuf(as_file_T* as) {
    sz_t size = INTREP_CHECKSTRLEN + // check str - \0
                sizeof(version_t) + // version
                sizeof(sz_t) + // libraries_size
                sizeof(flags_t); // flags
    char* buf = malloc(size);
    memcpy(buf, INTREP_CHECKSTR, INTREP_CHECKSTRLEN);
    size_t cur = INTREP_CHECKSTRLEN;
    do {
        version_t version; // 0.1
        version.major = 0;
        version.minor = 1;
        *((version_t*) &buf[cur]) = version;
        cur += sizeof(version_t);
    } while (0);
    do {
        sz_t libraries_size = 0;
        *((sz_t*) &buf[cur]) = libraries_size;
        cur += sizeof(sz_t);
    } while (0);
    flags_t* flags = (flags_t*) &buf[cur];
    cur += sizeof(flags_t);
    do {
        buf = realloc(buf, (size += sizeof(sz_t)));
        *((sz_t*) &buf[cur]) = as->data_size;
        cur += sizeof(sz_t);
        for (size_t i = 0; i < as->data_size; i++) {
            as_data_T* data = as->data[i];
            do {
                sz_t namelen = strlen(data->name);
                buf = realloc(buf, (size += sizeof(int32_t) + sizeof(sz_t) + namelen));
                *((int32_t*) &buf[cur]) = data->type;
                *((sz_t*) &buf[cur += sizeof(int32_t)]) = namelen;
                memcpy(&buf[cur += sizeof(sz_t)], data->name, namelen);
                cur += namelen;
            } while (0);
            switch (data->type) {
                case ASTYPE_CHAR: {
                    buf = realloc(buf, size += 1);
                    *((int8_t*) &buf[cur]) = data->value.char_value;
                    cur += 1;
                } break;
                case ASTYPE_SHORT: {
                    buf = realloc(buf, size += 2);
                    *((int16_t*) &buf[cur]) = data->value.short_value;
                    cur += 2;
                } break;
                case ASTYPE_INT: {
                    buf = realloc(buf, size += 4);
                    *((int32_t*) &buf[cur]) = data->value.int_value;
                    cur += 4;
                } break;
                case ASTYPE_LONG: {
                    buf = realloc(buf, size += 8);
                    *((int64_t*) &buf[cur]) = data->value.long_value;
                    cur += 8;
                    flags->needs_64 = 1;
                } break;
                case ASTYPE_STRING: {
                    sz_t len = strlen(data->value.ptr_value);
                    buf = realloc(buf, (size += sizeof(sz_t) + len));
                    *((sz_t*) &buf[cur]) = len;
                    memcpy(&buf[cur += sizeof(sz_t)], data->value.ptr_value, len);
                    cur += len;
                } break;
            }
        }
    } while (0);
    buf = realloc(buf, (size += sizeof(sz_t)));
    *((sz_t*) &buf[cur]) = as->functions_size;
    cur += sizeof(sz_t);
    for (size_t i = 0; i < as->functions_size; i++) {
        as_function_T* f = as->functions[i];
        do {
            sz_t namelen = strlen(f->name);
            buf = realloc(buf, (size += sizeof(sz_t) + namelen));
            *((sz_t*) &buf[cur]) = namelen;
            memcpy(&buf[cur += sizeof(sz_t)], f->name, namelen);
            cur += namelen;
        } while (0);
        do {
            buf = realloc(buf, (size += 4 * sizeof(sz_t)));
            *((sz_t*) &buf[cur]) = f->scope_size;
            *((sz_t*) &buf[cur += sizeof(sz_t)]) = f->argc;
            *((sz_t*) &buf[cur += sizeof(sz_t)]) = f->function_no;
            *((sz_t*) &buf[cur += sizeof(sz_t)]) = f->operations_size;
            cur += sizeof(sz_t);
        } while (0);
        do {
            for (size_t i = 0; i < f->operations_size; i++) {
                as_op_T* op = f->operations[i];
                if (op->op_size > 4 && op->type != ASOP_NEW) {
                    flags->needs_64 = 1;
                }
                switch (op->type) {
                    case ASOP_NOP: break;
                    case ASOP_FREEREG: case ASOP_POPREG: case ASOP_FREEPTRREG: case ASOP_PTRTOREG: {
                        buf = realloc(buf, (size += sizeof(int32_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        cur += sizeof(int32_t);
                    } break;
                    case ASOP_RETURN: case ASOP_NEXTREG: case ASOP_NEW: case ASOP_MEMTOREG: case ASOP_PUSHREG: case ASOP_LOCALMEMBMOD: {
                        buf = realloc(buf, (size += sizeof(int32_t) + sizeof(sz_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->op_size;
                        cur += sizeof(sz_t);
                    } break;
                    case ASOP_ARGTOSTACK: {
                        buf = realloc(buf, (size += sizeof(int32_t) + 3 * sizeof(sz_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->op_size;
                        *((sz_t*) &buf[cur += sizeof(sz_t)]) = op->argno;
                        *((sz_t*) &buf[cur += sizeof(sz_t)]) = op->var_location;
                        cur += sizeof(sz_t);
                    } break;
                    case ASOP_ARGTOREG: {
                        buf = realloc(buf, (size += sizeof(int32_t) + 2 * sizeof(sz_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->op_size;
                        *((sz_t*) &buf[cur += sizeof(sz_t)]) = op->argno;
                        cur += sizeof(sz_t);
                    } break;
                    case ASOP_SETLASTIMM: {
                        buf = realloc(buf, (size += sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((int32_t*) &buf[cur += sizeof(int32_t)]) = op->value.int_value;
                        *((int32_t*) &buf[cur += sizeof(int32_t)]) = op->data_type;
                        cur += sizeof(int32_t);
                    } break;
                    case ASOP_VMOD: {
                        buf = realloc(buf, (size += sizeof(int32_t) + 2 * sizeof(sz_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->op_size;
                        *((sz_t*) &buf[cur += sizeof(sz_t)]) = op->var_location;
                        cur += sizeof(sz_t);
                    } break;
                    case ASOP_VREF: {
                        buf = realloc(buf, (size += sizeof(int32_t) + sizeof(sz_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->var_location;
                        cur += sizeof(sz_t);
                    } break;
                    case ASOP_FCALL: case ASOP_SYMBADDRREF: case ASOP_SYMBOLREF: {
                        sz_t len = strlen(op->name);
                        buf = realloc(buf, (size += sizeof(int32_t) + sizeof(sz_t) + len));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = len;
                        memcpy(&buf[cur += sizeof(sz_t)], op->name, len);
                        cur += len;
                    } break;
                    case ASOP_RETVAL: case ASOP_RETNULL: case ASOP_LEA: {
                        buf = realloc(buf, (size += sizeof(int32_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        cur += sizeof(int32_t);
                    } break;
                    case ASOP_BINOP: {
                        buf = realloc(buf, (size += sizeof(int32_t) + sizeof(sz_t) + sizeof(int32_t) + sizeof(int32_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->op_size;
                        *((int32_t*) &buf[cur += sizeof(sz_t)]) = op->binop_type;
                        *((int32_t*) &buf[cur += sizeof(int32_t)]) = op->data_type;
                        cur += sizeof(int32_t);
                    } break;
                    case ASOP_MEMBREF: case ASOP_LOCALMEMB: {
                        buf = realloc(buf, (size += sizeof(int32_t) + sizeof(sz_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->memb_offset;
                        cur += sizeof(sz_t);
                    } break;
                    case ASOP_PTRMEMBMOD: {
                        buf = realloc(buf, (size += sizeof(int32_t) + 2 * sizeof(sz_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->op_size;
                        *((sz_t*) &buf[cur += sizeof(sz_t)]) = op->memb_offset;
                        cur += sizeof(sz_t);
                    } break;
                    case ASOP_JCOND: case ASOP_JMP: case ASOP_BB: {
                        buf = realloc(buf, (size += sizeof(int32_t) + sizeof(sz_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->bb_no;
                        cur += sizeof(sz_t);
                    } break;
                    case ASOP_UNOP: {
                        buf = realloc(buf, (size += sizeof(int32_t) + sizeof(sz_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->op_size;
                        *((int32_t*) &buf[cur += sizeof(sz_t)]) = op->unop_type;
                        cur += sizeof(int32_t);
                    } break;
                    case ASOP_STRINGREF: {
                        buf = realloc(buf, (size += sizeof(int32_t) + sizeof(sz_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((sz_t*) &buf[cur += sizeof(int32_t)]) = op->string_index;
                        cur += sizeof(sz_t);
                    } break;
                    case ASOP_SETLASTCMP: {
                        buf = realloc(buf, (size += sizeof(int32_t) + sizeof(int32_t)));
                        *((int32_t*) &buf[cur]) = op->type;
                        *((int32_t*) &buf[cur += sizeof(int32_t)]) = op->cmp_type;
                        cur += sizeof(int32_t);
                    } break;
                }
            }
        } while (0);
    }
    buf = realloc(buf, (size += sizeof(sz_t)));
    *((sz_t*) &buf[cur]) = as->unnamed_strings_size;
    cur += sizeof(sz_t);
    for (size_t i = 0; i < as->unnamed_strings_size; i++) {
        sz_t len = strlen(as->unnamed_strings[i]);
        buf = realloc(buf, (size += sizeof(sz_t) + len));
        *((sz_t*) &buf[cur]) = len;
        memcpy(&buf[cur += sizeof(sz_t)], as->unnamed_strings[i], len);
        cur += len;
    }
    sbuf_T sbuf;
    sbuf.buffer = buf;
    sbuf.size = size;
    return sbuf;
}