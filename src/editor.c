#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#include "matching.h"
#include "editor.h"
#include "file.h"
#include "undo.h"
#include "row.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define ENTER 13
#define ESC   27


char read_key() 
{
    char c; int readno;

    while ((readno = read(STDIN_FILENO, &c, 1)) != 1) {
        if (readno == -1 && errno == EAGAIN) 
            exit(1);
    }
    return c;
}

int get_cursor_position(int *x, int *y) 
{
    if (write(STDOUT_FILENO, "\x1b[6n", 4) == -1) 
        return -1;

    char buf[32];
    int i = 0;

    while (read(STDOUT_FILENO, &buf[i], 1) != -1) {
        if (buf[i] == 'R') 
            break;
        if (buf[i] != '\x1b' && buf[i] != '[') 
            i++;
    }
    buf[i] = '\0';

    if (sscanf(buf, "%d;%d", y, x) == -1)
        return -1;

    return 1;
}

int get_window_size(int *w, int *h) 
{
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) 
        return -1;
    get_cursor_position(w, h);
    write(STDOUT_FILENO, "\x1b[H", 3);
    return 1;
}

struct editor_state *editor_init() 
{   
    write(STDOUT_FILENO, "\x1b[2J", 4);
    struct editor_state *editor = malloc(sizeof(struct editor_state));
    tcgetattr(STDOUT_FILENO, &editor->init_term);
    editor->x = 0;
    editor->y = 0;
    editor->row = 0;
    editor->rows = malloc(sizeof(struct row*) * 64);
    editor->row_num = 0;
    editor->mode = M_NORMAL;
    editor->action_buf = ab_new();
    editor->edit_buf = ab_new();
    editor->status_buf = ab_new();
    editor->history = history_new();
    editor->last_update = time(NULL);
    enable_raw_mode();
    get_window_size(&editor->w_width, &editor->w_height);

    for (int i = 0; i < editor->w_height; i++) {
        editor_append_row(editor);
    }

    editor_set_status(editor, "Welcome to BADVIM v1.0");

    return editor;

}

void editor_process_key(struct editor_state *editor) 
{
    char c = read_key();
    switch(editor->mode)
    {
    case M_NORMAL:
        editor_add_edit(editor);
        switch(c) {
        case CURSOR_LEFT:
        case CURSOR_UP:
        case CURSOR_DOWN:
        case CURSOR_RIGHT:
            editor_move_cursor(editor, c);
            break;
        case N_QUIT:
            exit(1);
            break;
        case N_INSERT:
            editor->mode = M_INSERT;
            break;
        case N_DELETE:
            editor_delete_character(editor);
            editor_maybe_add_edit(editor, '\x7f');
            break;
        case N_SAVE:
        case N_OPEN:
        case N_PATTERN:
            editor->mode = M_ACTION;
            ab_append(editor->action_buf, &c, 1);
            break;
        case N_UNDO:
            editor_undo(editor);
            break;
        }
        break;
    case M_INSERT:
        switch (c) {
        case ESC:
            editor->mode = M_NORMAL;
            break;
        default:
            editor_insert_character(editor, c);
            editor_maybe_add_edit(editor, c);
            break;
        }
        break;
    case M_ACTION:
        switch(c) {
        case ENTER:
            editor_execute_action(editor);
            break;
        default:
            if (!iscntrl(c))
                ab_append(editor->action_buf, &c, 1);
        }
        break;
    }
}

void editor_refresh_screen(struct editor_state *editor) 
{
    struct append_buffer *out_buf = ab_new();

    ab_append(out_buf, "\x1b[?25l\x1b[H", 9);

    for (int i = 0; i < editor->w_height - 2; i++) {
        struct row *row = editor->rows[editor->row + i];
        ab_append(out_buf, row_extract_text(row, false), row->len);
        ab_append(out_buf, "\x1b[K", 3);
        ab_append(out_buf, "\r\n", 2);
    }

    ab_append(out_buf, "ACTION: ", 8);
    ab_append(out_buf, editor->action_buf->buffer, editor->action_buf->len);
    ab_append(out_buf, "\x1b[K", 3);
    ab_append(out_buf, "\r\n", 2);

    ab_append(out_buf, "STATUS: ", 8);
    ab_append(out_buf, editor->status_buf->buffer, editor->status_buf->len);
    ab_append(out_buf, "\x1b[K", 3);


    char pos_buf[32];

    struct row *row = editor->rows[editor->y];
    snprintf(pos_buf, sizeof(pos_buf), "\x1b[%d;%dH", editor->y + 1 - editor->row, MIN(editor->x, row->len));

    ab_append(out_buf, pos_buf, strlen(pos_buf));
    ab_append(out_buf, "\x1b[?25h", 6);

    write(STDOUT_FILENO, out_buf->buffer, out_buf->len);

    ab_free(out_buf);
}

void editor_append_row(struct editor_state *editor) 
{
    editor->rows[editor->row_num++] = row_new();
}

void enable_raw_mode() 
{
    struct termios term;
    tcgetattr(STDOUT_FILENO, &term);
    term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    term.c_cflag &= ~OPOST;
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &term);
}

void editor_move_cursor(struct editor_state *editor, char c)
{   
    struct row *row = editor->rows[editor->y];

    switch (c) {
    case CURSOR_UP:
        if (editor->y > 0)
            editor->y--;
        break;
    case CURSOR_DOWN:
        editor->y++;
        break;
    case CURSOR_LEFT:
        editor->x = MAX(0, MIN(row->len, editor->x - 1));
        if (editor->x == 0 && editor->y > 0) 
            editor->x = editor->rows[--editor->y]->len;
        break;
    case CURSOR_RIGHT:
        editor->x = MIN(row->len, editor->x + 1);
        if (editor->x == row->len) {
            editor->y++;
            editor->x = 0;
        }
        break;
    }
    if (editor->y - editor->row == editor->w_height - 2) {
        editor->row++;
        if (editor->y == editor->row_num)
            editor_append_row(editor);
    }
    if (editor->y < editor->row)
        editor->row--;
}

void editor_insert_character(struct editor_state *editor, char c)
{
    if (!iscntrl(c)) {
        struct row *row = editor->rows[editor->y];
        size_t x = MIN(editor->x, row->len);
        row_insert_character(row, c, x);
        editor->x++;
    }
}

void editor_delete_character(struct editor_state *editor)
{
    struct row *row = editor->rows[editor->y];

    if (row->len != 0)  {
        size_t x = MIN(editor->x, row->len);
        row_delete_character(row, x);
        editor->x--;
    }
}

void editor_execute_action(struct editor_state *editor)
{
    char action;
    char *arg = malloc(64);

    char *buf = editor->action_buf->buffer;
    action = *buf;

    switch (action) {
    case N_PATTERN:
        editor->mode = M_SEARCH;
        strncpy(arg, buf + 1, editor->action_buf->len - 1);
        arg[editor->action_buf->len - 1] = '\0';
        editor_search_mode(editor, arg);
        break;
    case N_SAVE:
        strncpy(arg, buf + 2, editor->action_buf->len - 2);
        arg[editor->action_buf->len - 2] = '\0';
        if (save_to_file(editor, arg) == -1)
            editor_set_status(editor, "Couldn't save to file.");
        break;
    case N_OPEN:
        editor_flush_buffer(editor);  
        strncpy(arg, buf + 2, editor->action_buf->len - 2);
        arg[editor->action_buf->len - 2] = '\0';
        if (pipe_file_to_editor(editor, arg) == -1)
            editor_set_status(editor, "Couldn't load file.");
        break;
    }

    editor->action_buf->len = 0;
    editor->mode = M_NORMAL;

    free(arg);
}

void editor_flush_buffer(struct editor_state *editor)
{   
    for (int i = 0; i < editor->row_num; i++) {
        struct row *row = editor->rows[i];
        row_flush_buffer(row);
    }
}

void editor_maybe_add_edit(struct editor_state *editor, char c)
{
    if (c == '\x7f') {
        struct row *row = editor->rows[editor->y];
        char *buf = row_extract_text(row, false);
        ab_append(editor->edit_buf, &buf[editor->x], 1);
    } else {
        c = '\x7f';
        ab_append(editor->edit_buf, &c, 1);
    }

    time_t now = time(NULL);
    
    if (now > editor->last_update && editor->edit_buf->len != 0) 
        editor_add_edit(editor);
    
    editor->last_update = now;
}

void editor_add_edit(struct editor_state *editor)
{
    if (editor->edit_buf->len != 0) {
        history_append_edit(editor, editor->history, ab_extract_text(editor->edit_buf), editor->x, editor->y);
        editor->edit_buf->len = 0;
    }
}

void editor_undo(struct editor_state *editor)
{
    if (editor->history->len == 0) return;
    struct edit* edit = history_get_edit(editor->history);
    editor->x = edit->x;
    editor->y = edit->y;
    if (editor->row > editor->y)
        editor->row = editor->y;
    for (int i = strlen(edit->buffer) - 1; i >= 0; i--) {
        if (edit->buffer[i] == '\x7f') {
            editor_delete_character(editor);
            editor->x++;
        } else {
            editor_insert_character(editor, edit->buffer[i]);
            editor->x--;
        }
    }

    history_remove_edit(editor->history);

}

void editor_search_mode(struct editor_state *editor, char *pattern)
{
    struct search_node *search = scan_for_occurence(editor, pattern);
    struct search_node *ptr = search;
    bool exit = false;

    while (1) {

        if (ptr != NULL) {
            editor->x = ptr->x;
            editor->y = ptr->y;
        }
      
        char c = read_key();
        
        if (exit)
            break;

        switch(c) {
        case ESC:
            exit = true;
            break;
        case CURSOR_LEFT:
        case CURSOR_UP:
            if (ptr != NULL && ptr->prev != NULL)
                ptr = ptr->prev;
            break;
        case CURSOR_RIGHT:
        case CURSOR_DOWN:
            if (ptr != NULL && ptr->next != NULL)
                ptr = ptr->next;
            break;
        }
        editor_refresh_screen(editor);
    }

    editor->mode = M_NORMAL;

}

void editor_set_status(struct editor_state *editor, char *str)
{
    editor->status_buf->len = 0;
    ab_append(editor->status_buf, str, strlen(str));
}