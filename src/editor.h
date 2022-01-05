#ifndef __EDITOR_H__
#define __EDITOR_H__


#include <termios.h>

#include "buffer.h"

struct Row {
    struct Buffer *buf;
};

struct Row* row_init(const char* str);
void row_free(struct Row* row);

struct Editor {
    int             x, y;
    int             w_width, w_height;
    int             row_num;
    struct termios  init_term;
    struct Row      **rows;
};

struct Editor* editor_init();
void editor_append_row(struct Editor *editor, const char* str);
void editor_refresh_screen(struct Editor *editor);

char read_key();
void editor_process_key();

void enable_raw_mode();
void disable_raw_mode();

#endif