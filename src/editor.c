#include <unistd.h>
#include <termio.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>


#include "editor.h"

struct Row* row_init(const char* str) {
    struct Row *row = malloc(sizeof *row);
    row->buf = buffer_init();
    buffer_append(row->buf, "~", 1);
    buffer_append(row->buf, str, strlen(str));
    buffer_append(row->buf, "\x1b[K", 3);
    return row;
}

void row_free(struct Row *row) {
    buffer_free(row->buf);
    free(row);
}

void row_insert_char(struct Row *row, int x, const char c) {
    x = x > row->buf->len ? row->buf->len: x;
    if (x == row->buf->len)
}

char* row_render(struct Row *row) {
    return row->buf->buf;
}


char read_key() {
    char c; int readno;
    while ((readno = read(STDIN_FILENO, &c, 1)) != 1) {
        if (readno == -1 && errno == EAGAIN) 
            exit(1);
    }
    return c;
}

int get_cursor_position(int *x, int *y) {
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

int get_window_size(int *w, int *h) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) 
        return -1;
    get_cursor_position(w, h);
    write(STDOUT_FILENO, "\x1b[H", 3);
    return 1;
}

struct Editor* editor_init() {
    struct Editor* editor = malloc(sizeof *editor);
    tcgetattr(STDOUT_FILENO, &editor->init_term);
    editor->x = 0;
    editor->y = 0;
    editor->rows = malloc(sizeof(struct Row*) * 64);
    editor->row_num = 0;
    enable_raw_mode();
    get_window_size(&editor->w_width, &editor->w_height);
    for (int i = 0; i < editor->w_height; i++) {
        editor_append_row(editor, "");
    }

    return editor; 
}

void editor_process_key(struct Editor *editor) {
    char c = read_key();
    switch (c) {
        case 'q':
            exit(1);
            break;
        case 'w':
            if (editor->y > 0) 
                editor->y--;
            break;
        case 'a':
            if (editor->x > 0) 
                editor->x--;
            break;
        case 's':
            if (editor->y < editor->w_height) 
                editor->y++;
            break;
        case 'd':
            if (editor->x < editor->w_width) 
                editor->x++;
            break;
        default:
            row_insert_char(editor->rows[editor->y + 1], editor->x, c);
    }
}

void editor_refresh_screen(struct Editor* editor) {
    struct Buffer* out_buf = buffer_init();
    buffer_append(out_buf, "\x1b[?25l\x1b[H", 9);
    for (int i = 0; i < editor->w_height; i++) {
        struct Row *row = editor->rows[i];
        buffer_append(out_buf, row_render(row), row->buf->len);
        if (i < editor->w_height - 1) 
            buffer_append(out_buf, "\r\n", 5);
    }
    
    char pos_buf[32];
    snprintf(pos_buf, sizeof(pos_buf), "\x1b[%d;%dH", editor->y, editor->x);

    buffer_append(out_buf, pos_buf, strlen(pos_buf));
    buffer_append(out_buf, "\x1b[?25h", 6);
    write(STDOUT_FILENO, out_buf->buf, out_buf->len);
    buffer_free(out_buf);
}

void editor_append_row(struct Editor *editor, const char* str) {
    editor->rows[editor->row_num++] = row_init(str);
}

void enable_raw_mode() {
    struct termios term;
    tcgetattr(STDOUT_FILENO, &term);
    term.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    term.c_cflag &= ~OPOST;

    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &term);
}