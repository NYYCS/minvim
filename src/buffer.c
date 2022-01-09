#include <stdlib.h>
#include <string.h>

#include "buffer.h"

struct append_buffer* ab_new() 
{
    struct append_buffer *buf = malloc(sizeof(struct append_buffer));
    buf->buffer  = malloc(INIT_BUFSIZ);
    buf->size = INIT_BUFSIZ;
    buf->len  = 0;
    return buf;
}

void ab_free(struct append_buffer *buf) 
{
    free(buf->buffer);
    free(buf);
}

void ab_resize(struct append_buffer *buf, uint size) 
{
    buf->buffer = realloc(buf->buffer, size);
    buf->size = size;
}

void ab_append(struct append_buffer *buf, char *str, uint len) 
{
    while (buf->len + len > buf->size) {
        ab_resize(buf, 2 * buf->size);
    }
    memmove(buf->buffer + buf->len, str, len);
    buf->len += len;
}

char* ab_extract_text(struct append_buffer *buf)
{
    char *ret = malloc(buf->len + 1);
    strncpy(ret, buf->buffer, buf->len);
    ret[buf->len] = '\0';
    
    return ret;
}