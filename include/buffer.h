#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdlib.h>
#include <stddef.h>

#define INIT_BUFSIZ 1024

#define uint unsigned int

struct append_buffer {
    char *buffer;
    uint  size;
    uint  len;
};

struct append_buffer *ab_new();
void  ab_free(struct append_buffer *buf);
void  ab_resize(struct append_buffer *buf, uint size);
void  ab_append(struct append_buffer *buf, char *str, uint len);
char *ab_extract_text(struct append_buffer *buf);

#endif