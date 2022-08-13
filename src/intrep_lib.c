#include "include/intrep_internal.h"
#include "include/utils.h"
#include <string.h>
#include <stdlib.h>

void intrep_write_typeptr(data_type_T* t, wrapper_t wrapper) {
    for (;;) {
        writei(wrapper, t->primitive_type);
        writesz(wrapper, t->primitive_size);
        if (t->primitive_type == TYPE_PTR) {
        } else if (t->primitive_type == TYPE_ARRAY) {
            writesz(wrapper, t->array_count);
        } else {
            write_string(wrapper, t->type_name);
            return;
        }
        t = t->ptr_type;
    }
}

void intrep_write_fspec(fspec_T* f, wrapper_t wrapper) {
    write_string(wrapper, f->name);
    write_string(wrapper, f->symbol_name);
    writei(wrapper, f->flags);
    intrep_write_typeptr(f->return_type, wrapper);
    writesz(wrapper, f->unnamed_length);
    for (size_t i = 0; i < f->unnamed_length; i++) {
        intrep_write_typeptr(f->unnamed_types[i], wrapper);
    }
    writesz(wrapper, f->named_length);
    for (size_t i = 0; i < f->named_length; i++) {
        write_string(wrapper, f->named_names[i]);
        intrep_write_typeptr(f->named_types[i], wrapper);
    }
}
void intrep_write_data_type(data_type_T* t, wrapper_t wrapper, fspec_T*** fspecs, size_t* fspecs_size) {
    writei(wrapper, t->primitive_type);
    writesz(wrapper, t->primitive_size);
    if (t->primitive_type == TYPE_STRUCT) {
        write_string(wrapper, t->type_name);
        writesz(wrapper, t->instance_members_size);
        for (size_t i = 0; i < t->instance_members_size; i++) {
            data_type_T* m = t->instance_member_types[i];
            write_string(wrapper, t->instance_member_names[i]);
            intrep_write_typeptr(m, wrapper);
            writei(wrapper, m->flags);
        }
        writesz(wrapper, t->static_members_size);
        for (size_t i = 0; i < t->static_members_size; i++) {
            data_type_T* m = t->static_member_types[i];
            write_string(wrapper, t->static_member_names[i]);
            intrep_write_typeptr(m, wrapper);
            writei(wrapper, m->flags);
        }
        writesz(wrapper, t->instance_functions_size);
        for (size_t i = 0; i < t->instance_functions_size; i++) {
            fspec_T* f = t->instance_functions[i];
            intrep_write_fspec(f, wrapper);
            list_add(fspecs, fspecs_size, f);
        }
        writesz(wrapper, t->static_functions_size);
        for (size_t i = 0; i < t->static_functions_size; i++) {
            fspec_T* f = t->static_functions[i];
            intrep_write_fspec(f, wrapper);
            list_add(fspecs, fspecs_size, f);
            
        }
    }
}

void intrep_write_lib(as_file_T* as, global_T* global, wrapper_t wrapper) {
    wrapper_write(INTREP_CHECKSTR, INTREP_CHECKSTRLEN, wrapper);
    do {
        version_t version; // 0.1
        version.major = 0;
        version.minor = 1;
        printf("version %0llx %d %x\n", version.version, version.major, version.minor);
        wrapper_write(&version, sizeof(version), wrapper);
    } while (0);
    writei(wrapper, INTREP_LIB);

    fspec_T** fspecs = (void*) 0;
    size_t fspecs_size = 0;
    for (size_t i = 0; i < global->functions_size; i++) {
        list_add(&fspecs, &fspecs_size, global->function_specs[i]);
    }

    flags_t flags;
    off_t flagspos = wrapper_seek(sizeof(flags_t), SEEK_CUR, wrapper) - sizeof(flags_t);

    off_t type_count_location = wrapper_seek(sizeof(sz_t), SEEK_CUR, wrapper) - sizeof(sz_t);
    sz_t type_count = 0;
    for (size_t i = 0; i < global->types_size; i++) {
        data_type_T* t = global->types[i];
        if (t->is_fromsource) {
            type_count++;
            intrep_write_data_type(t, wrapper, &fspecs, &fspecs_size);
        }
    }
    off_t cur = wrapper_seek(0, SEEK_CUR, wrapper);
    wrapper_seek(type_count_location, SEEK_SET, wrapper);
    writesz(wrapper, type_count);
    wrapper_seek(cur, SEEK_SET, wrapper);

    writesz(wrapper, fspecs_size);
    for (size_t i = 0; i < fspecs_size; i++) {
        fspec_T* f = fspecs[i];
        intrep_write_fspec(f, wrapper);
    }

    writesz(wrapper, as->functions_size);
    for (size_t i = 0; i < as->functions_size; i++) {
        intrep_write_as_function(wrapper, as->functions[i], &flags);
    }
    cur = wrapper_seek(0, SEEK_CUR, wrapper);
    wrapper_seek(flagspos, SEEK_SET, wrapper);
    wrapper_write(&flags, sizeof(flags_t), wrapper);
    wrapper_seek(cur, SEEK_SET, wrapper);
}


data_type_T* intrep_read_typeptr(wrapper_t wrapper) {
    data_type_T* t = calloc(1, sizeof(data_type_T));
    data_type_T* m = t;
    for (;;) {
        m->primitive_type = readi(wrapper);
        m->primitive_size = readsz(wrapper);
        if (t->primitive_type == TYPE_PTR) {
            m->ptr_type = calloc(1, sizeof(data_type_T));
            m = m->ptr_type;
        } else if (t->primitive_type == TYPE_ARRAY) {
            m->array_count = readsz(wrapper);
            m->ptr_type = calloc(1, sizeof(data_type_T));
            m = m->ptr_type;
        } else {
            m->type_name = read_string(wrapper);
            return t;
        }
    }
}

fspec_T* intrep_read_fspec(wrapper_t wrapper) {
    fspec_T* f = calloc(1, sizeof(fspec_T));
    f->name = read_string(wrapper);
    f->symbol_name = read_string(wrapper);
    f->flags = readi(wrapper);
    f->return_type = intrep_read_typeptr(wrapper);
    f->unnamed_length = readsz(wrapper);
    f->unnamed_types = calloc(1, f->unnamed_length * sizeof(data_type_T*));
    for (size_t i = 0; i < f->unnamed_length; i++) {
        f->unnamed_types[i] = intrep_read_typeptr(wrapper);
    }
    f->named_length = readsz(wrapper);
    f->named_types = calloc(1, f->named_length * sizeof(data_type_T*));
    f->named_names = calloc(1, f->named_length * sizeof(char*));
    for (size_t i = 0; i < f->named_length; i++) {
        f->named_names[i] = read_string(wrapper);
        f->named_types[i] = intrep_read_typeptr(wrapper);
    }
    return f;
}
data_type_T* intrep_read_data_type(wrapper_t wrapper) {
    data_type_T* t = calloc(1, sizeof(data_type_T));
    t->primitive_type = readi(wrapper);
    t->primitive_size = readsz(wrapper);
    if (t->primitive_type == TYPE_STRUCT) {
        t->type_name = read_string(wrapper);
        t->instance_members_size = readsz(wrapper);
        t->instance_member_types = calloc(1, t->instance_members_size * sizeof(data_type_T*));
        t->instance_member_names = calloc(1, t->instance_members_size * sizeof(char*));
        for (size_t i = 0; i < t->instance_members_size; i++) {
            t->instance_member_names[i] = read_string(wrapper);
            data_type_T* m = (t->instance_member_types[i] = intrep_read_typeptr(wrapper));
            m->flags = readi(wrapper);
        }
        t->static_members_size = readsz(wrapper);
        for (size_t i = 0; i < t->static_members_size; i++) {
            t->static_member_names[i] = read_string(wrapper);
            data_type_T* m = (t->static_member_types[i] = intrep_read_typeptr(wrapper));
            m->flags = readi(wrapper);
        }
        t->instance_functions_size = readsz(wrapper);
        t->instance_functions = calloc(1, t->instance_functions_size * sizeof(fspec_T*));
        for (size_t i = 0; i < t->instance_functions_size; i++) {
            t->instance_functions[i] = intrep_read_fspec(wrapper);
        }
        t->static_functions_size = readsz(wrapper);
        t->static_functions = calloc(1, t->static_functions_size * sizeof(fspec_T*));
        for (size_t i = 0; i < t->static_functions_size; i++) {
            t->static_functions[i] = intrep_read_fspec(wrapper);
        }
    }
    return t;
}
data_type_T* intrep_resolve_type(data_type_T* t, global_T* global);
fspec_T* intrep_resolve_fspec(fspec_T* fspec, global_T* global) {
    fspec->return_type = intrep_resolve_type(fspec->return_type, global);
    for (size_t i = 0; i < fspec->unnamed_length; i++) {
        fspec->unnamed_types[i] = intrep_resolve_type(fspec->unnamed_types[i], global);
    }
    for (size_t i = 0; i < fspec->named_length; i++) {
        fspec->named_types[i] = intrep_resolve_type(fspec->named_types[i], global);
    }
    return fspec;
}
data_type_T* intrep_resolve_type(data_type_T* t, global_T* global) {
    if (t->primitive_type == TYPE_PTR) {
        t->ptr_type = intrep_resolve_type(t->ptr_type, global);
    } else if (t->primitive_type == TYPE_ARRAY) {
        t->ptr_type = intrep_resolve_type(t->ptr_type, global);
    } else if (t->primitive_type == TYPE_STRUCT) {
        for (size_t i = 0; i < t->instance_members_size; i++) {
            t->instance_member_types[i] = intrep_resolve_type(t->instance_member_types[i], global);
        }
        for (size_t i = 0; i < t->static_members_size; i++) {
            t->static_member_types[i] = intrep_resolve_type(t->static_member_types[i], global);
        }
        for (size_t i = 0; i < t->instance_functions_size; i++) {
            t->instance_functions[i] = intrep_resolve_fspec(t->instance_functions[i], global);
        }
        for (size_t i = 0; i < t->static_functions_size; i++) {
            t->static_functions[i] = intrep_resolve_fspec(t->static_functions[i], global);
        }
    } else {
        char* type_name = t->type_name;
        free(t);
        t = scope_get_typeg(global, type_name);
        free(type_name);
    }
    return t;
}

void intrep_read_lib(as_file_T* as, global_T* global, wrapper_t wrapper) {
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
    if (mode != INTREP_LIB) {
        return;
    }

    flags_t flags;
    wrapper_read(&flags, sizeof(flags_t), wrapper);
    sz_t type_count = readsz(wrapper);
    data_type_T** types = calloc(1, type_count * sizeof(data_type_T*));
    for (size_t i = 0; i < type_count; i++) {
        types[i] = intrep_read_data_type(wrapper);
    }
    for (size_t i = 0; i < type_count; i++) {
        scope_add_typeg(global, intrep_resolve_type(types[i], global));
    }
    sz_t fspecs_size = readsz(wrapper);
    fspec_T** fspecs = calloc(1, fspecs_size * sizeof(fspec_T*));
    for (size_t i = 0; i < fspecs_size; i++) {
        fspecs[i] = intrep_read_fspec(wrapper);
    }
    for (size_t i = 0; i < fspecs_size; i++) {
        scope_add_functiong(global, intrep_resolve_fspec(fspecs[i], global));
    }

    sz_t as_functions_size = readsz(wrapper);
    for (size_t i = 0; i < as_functions_size; i++) {
        as_add_function(as, intrep_read_as_function(wrapper));
    }
}