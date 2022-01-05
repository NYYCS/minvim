#include <stdlib.h>
#include <string.h>

#include "buffer.h"

struct Buffer* buffer_init() {
    struct Buffer *buf = malloc(sizeof *buf);
    buf->buf  = malloc(INIT_BUFSIZ);
    buf->size = INIT_BUFSIZ;
    buf->len  = 0;
    return buf;
}

void buffer_free(struct Buffer *buf) {
    free(buf->buf);
    free(buf);
}

void buffer_resize(struct Buffer *buf, const uint size) {
    buf->buf = realloc(buf->buf, size);
    buf->size = size;
}

void buffer_append(struct Buffer *buf, const char *str, const uint len) {
    while (buf->len + len > buf->size) {
        buffer_resize(buf, 2 * buf->size);
    }
    memmove(buf->buf + buf->len, str, len);
    buf->len += len;
}

