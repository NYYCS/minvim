#ifndef __ROW_H__
#define __ROW_H__

#include <stddef.h>
#include <stdbool.h>

#define uint unsigned int

#define BUFSIZ 1024

struct gap_buffer {
    uint   size;
    uint   start;  
    uint   end;
    uint   len;
    char  *buffer;
};

struct gap_buffer *gb_new();
void  gb_free(struct gap_buffer *buf);
void  gb_grow(struct gap_buffer *buf, uint new_size);
void  gb_shrink(struct gap_buffer *buf, uint new_size);
void  gb_insert_character(struct gap_buffer *buf, char c);
void  gb_delete_character(struct gap_buffer *buf);
char *gb_extract_text(struct gap_buffer *buf, bool terminate);

struct row {
    uint               len;
    bool               modified;
    struct gap_buffer *buffer;         
};

struct row *row_new();
void  row_free(struct row *row);
void  row_insert_character(struct row *row, char c, uint x);
void  row_delete_character(struct row *row, uint x);
char *row_extract_text(struct row *row, bool terminate);
void  row_flush_buffer(struct row *row);


#endif