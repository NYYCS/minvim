
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "row.h"

struct gap_buffer *gb_new()
{
    struct gap_buffer *buf = malloc(sizeof(struct gap_buffer));
    buf->size   = BUFSIZ;
    buf->start  = 0;
    buf->end    = BUFSIZ;
    buf->len    = 0;
    buf->buffer = malloc(BUFSIZ);
    
    return buf;
}

void gb_free(struct gap_buffer *buf)
{
    free(buf->buffer);
    free(buf);
}

#define gb_front(buf) ((buf)->start)
#define gb_back(buf)  (((buf)->size) - ((buf)->end))

void move_back(struct gap_buffer *buf, char *new_buf, uint new_size)
{
    memmove(new_buf + new_size - gb_back(buf), buf->buffer + buf->end, gb_back(buf));
}

void gb_grow(struct gap_buffer *buf, uint new_size)
{
    char *new_buf = realloc(buf->buffer, new_size);
    move_back(buf, new_buf, new_size);
    buf->size   = new_size;
    buf->end    = new_size - gb_back(buf);
    buf->buffer = new_buf;
}

void gb_shrink(struct gap_buffer *buf, uint new_size)
{
    move_back(buf, buf->buffer, new_size);
    buf->end  = new_size - gb_back(buf);
    buf->size = new_size;
    char *new_buf = realloc(buf->buffer, new_size);
    buf->buffer = new_buf;
}

void cursor_left(struct gap_buffer *buf)
{
    if (buf->start > 0) 
        buf->buffer[--buf->end] = buf->buffer[--buf->start];
}

void cursor_right(struct gap_buffer *buf)
{
    if (buf->start < buf->size) 
        buf->buffer[++buf->start] = buf->buffer[++buf->end];
}

void gb_insert_character(struct gap_buffer *buf, char c)
{
    if (buf->start == buf->end)
        gb_grow(buf, 2 * buf->size);
    
    buf->buffer[buf->start++] = c;
    buf->len++;
}

void gb_delete_character(struct gap_buffer *buf) 
{
    if (buf->end < buf->size) {
        buf->end++;
        buf->len++;
    }

    if (buf->len < buf->size / 4) 
        gb_shrink(buf, buf->size / 2);
}

char *gb_extract_text(struct gap_buffer *buf, bool terminate)
{
    char *text = malloc(buf->len + 1);
    strncpy(text, buf->buffer, buf->start);
    strncpy(text + buf->start, buf->buffer + buf->end, gb_back(buf));
    if (terminate)
        text[buf->len] = '\0';
    return text;
}

struct row *row_new()
{
    struct row *row = malloc(sizeof(struct row));
    row->len = 0;
    row->modified = false;
    row->buffer = gb_new();  
    return row;
}

void row_free(struct row *row)
{
    gb_free(row->buffer);
    free(row);
}

void row_insert_character(struct row *row, char c, uint x)
{
    struct gap_buffer *buf = row->buffer;

    while (buf->start != x) {
        if (buf->start > x)
            cursor_left(buf);
        else
            cursor_right(buf);
    }

    gb_insert_character(buf, c);

    row->modified = true;
    row->len++;
}

void row_delete_character(struct row *row, uint x)
{
    struct gap_buffer *buf = row->buffer;

    while (buf->start != x) {
        if (buf->start > x)
            cursor_left(buf);
        else
            cursor_right(buf);
    }
    
    gb_delete_character(buf);

    row->modified = true;
    row->len--;
}

char *row_extract_text(struct row *row, bool terminate)
{  
    return gb_extract_text(row->buffer, terminate);
}

void row_flush_buffer(struct row *row)
{
    struct gap_buffer *buf = row->buffer;
    gb_shrink(buf, BUFSIZ);
    buf->start = 0;
    buf->end   = buf->size;
    buf->len   = 0;
    row->len   = 0;
}