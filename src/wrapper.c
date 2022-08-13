#include "include/wrapper.h"
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

static int __wrapper_errno;
int *__wrapper_error() {
    return &__wrapper_errno;
}

struct WRAPPER {
    wrapper_readfn readfn;
    wrapper_writefn writefn;
    wrapper_seekfn seekfn;
    wrapper_closefn closefn;
    void* user_param;
};

size_t wrapper_fread(void* buf, size_t size, void* file) {
    int ret = fread(buf, size, 1, (FILE*) file);
    if (ret) {
        __wrapper_errno = errno;
    }
    return ret;
}
size_t wrapper_fwrite(void* buf, size_t size, void* file) {
    int ret = fwrite(buf, size, 1, (FILE*) file);
    if (ret) {
        __wrapper_errno = errno;
    }
    return ret;
}
off_t wrapper_fseek(off_t offset, int whence, void* file) {
    if (fseek((FILE*) file, offset, whence) < 0) {
        __wrapper_errno = errno;
        return -1;
    }
    int ret = ftell(file);
    if (ret < 0) {
        __wrapper_errno = errno;
    }
    return ret;
}
int wrapper_fclose(void* file) {
    int ret = fclose((FILE*) file);
    if (ret) {
        __wrapper_errno = errno;
    }
    return ret;
}

wrapper_t wrapper_wrapf(FILE* file) {
    if (file == (void*) 0) {
        
        return (void*) 0;
    }
    int ret = fflush(file); // flush file
    if (ret == EOF) {
        __wrapper_errno = errno;
        return (void*) 0;
    }
    wrapper_t wrapper = malloc(sizeof(*wrapper));
    wrapper->readfn = wrapper_fread;
    wrapper->writefn = wrapper_fwrite;
    wrapper->seekfn = wrapper_fseek;
    wrapper->closefn = wrapper_fclose;
    wrapper->user_param = file;
    return wrapper;
}

struct BUFDATA {
    char** buffer;
    size_t* size;
    off_t cur;
};

size_t wrapper_bufread(void* buf, size_t size, void* param) {
    #define data ((struct BUFDATA*) param)
    size_t toread;
    if (data->cur + size > *data->size) {
        toread = *data->size - data->cur;
    } else {
        toread = size;
    }
    memcpy(buf, &(*data->buffer)[data->cur], toread);
    data->cur += size;
    return toread;
}
size_t wrapper_bufwrite(void* buf, size_t size, void* param) {
    if (data->cur + size > *data->size) {
        *data->buffer = realloc(*data->buffer, *data->size = (size + data->cur));
    }
    memcpy(&(*data->buffer)[data->cur], buf, size);
    data->cur += size;
    return size;
}
off_t wrapper_bufseek(off_t offset, int whence, void* param) {
    if (whence == SEEK_SET) {
        data->cur = offset;
    } else if (whence == SEEK_CUR) {
        data->cur += offset;
    } else if (whence == SEEK_END) {
        data->cur = *data->size + offset;
    }
    if (data->cur > *data->size) {
        size_t oldsize = *data->size;
        *data->buffer = realloc(*data->buffer, *data->size = data->cur);
        memset(&(*data->buffer)[oldsize], 0, data->cur - oldsize);
    }

    return data->cur;
    #undef data
}
int wrapper_bufclose(void* param) {
    free(param);
    return 0;
}

wrapper_t wrapper_wrapbuf(char** buffer, size_t* size) {
    wrapper_t wrapper = malloc(sizeof(struct WRAPPER));
    wrapper->user_param = malloc(sizeof(struct BUFDATA));
    ((struct BUFDATA*) wrapper->user_param)->buffer = buffer;
    ((struct BUFDATA*) wrapper->user_param)->size = size;
    ((struct BUFDATA*) wrapper->user_param)->cur = 0;
    wrapper->readfn = wrapper_bufread;
    wrapper->writefn = wrapper_bufwrite;
    wrapper->seekfn = wrapper_bufseek;
    wrapper->closefn = wrapper_bufclose;
    return wrapper;
}

int wrapper_read(void* buf, size_t size, wrapper_t wrapper) {
    return wrapper->readfn(buf, size, wrapper->user_param);
}
int wrapper_write(void* buf, size_t size, wrapper_t wrapper) {
    return wrapper->writefn(buf, size, wrapper->user_param);
}
int wrapper_seek(off_t off, int mode, wrapper_t wrapper) {
    return wrapper->seekfn(off, mode, wrapper->user_param);
}
int wrapper_close(wrapper_t wrapper) {
    int ret = wrapper->closefn(wrapper->user_param);
    free(wrapper);
    return ret;
}