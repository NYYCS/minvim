#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdlib.h>
#include <stddef.h>

#define INIT_BUFSIZ 1024

#define uint unsigned int

struct Buffer {
    char *buf;
    uint  size;
    uint  len;
};

struct Buffer* buffer_init();
void buffer_free(struct Buffer *buf);
void buffer_resize(struct Buffer *buf, const uint size);
void buffer_append(struct Buffer *buf, const char *str, const uint len);

#endif