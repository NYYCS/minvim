#ifndef __EDITOR_H__
#define __EDITOR_H__


#include <termios.h>
#include <time.h>

#include "buffer.h"
#include "undo.h"
#include "row.h"

#define uint unsigned int


enum editor_mode {
    M_NORMAL,
    M_INSERT,
    M_ACTION,
    M_SEARCH
};

enum normal_action {
    N_INSERT  = 'i',
    N_QUIT    = 'q',
    N_DELETE  = 'x',
    N_PATTERN = '/',
    N_SAVE    = 'w',
    N_OPEN    = 'o',
    N_UNDO    = 'u'
};

enum cursor_key {
    CURSOR_LEFT  = 'h',
    CURSOR_UP    = 'j',
    CURSOR_DOWN  = 'k',
    CURSOR_RIGHT = 'l'
};

struct editor_state {
    int                    x, y;
    int                    row, col;
    int                    w_width, w_height;
    int                    row_num;
    struct termios         init_term;
    struct row           **rows;
    enum editor_mode       mode;
    char                   status[128];
    struct append_buffer  *action_buf;
    struct append_buffer  *edit_buf;
    struct append_buffer  *status_buf;
    time_t                 last_update;
    struct history        *history;
};

struct editor_state *editor_init();
void editor_append_row(struct editor_state *editor);
void editor_refresh_screen(struct editor_state *editor);
void editor_process_key(struct editor_state *editor);
void editor_move_cursor(struct editor_state *editor, char c);
void editor_insert_character(struct editor_state *editor, char c);
void editor_delete_character(struct editor_state *editor);
void editor_execute_action(struct editor_state *editor);
void editor_flush_buffer(struct editor_state *editor);
void editor_search_mode(struct editor_state *editor, char *pattern);
void editor_maybe_add_edit(struct editor_state *editor, char c);
void editor_add_edit(struct editor_state *editor);
void editor_undo(struct editor_state *editor);
void editor_set_status(struct editor_state *editor, char *str);

char read_key();
void enable_raw_mode();  
void disable_raw_mode();

#endif