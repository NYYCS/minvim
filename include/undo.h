#ifndef __UNDO_H__
#define __UNDO_H__

#include "editor.h"

#define uint unsigned int

#define H_MAX_LEN 64

struct editor_state;

struct edit {
    char *buffer;
    int   x, y;
    uint  len;
};

struct edit *edit_new(char *str, int x, int y);

struct history {
    uint          len;
    uint          counter;
    struct edit **edits; 
};

struct history *history_new();
void history_append_edit(struct editor_state *editor, struct history *hist, char *str, int x, int y);
void history_remove_edit(struct history *hist);
struct edit *history_get_edit(struct history *hist);

#endif