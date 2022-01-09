#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "editor.h"
#include "undo.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


struct edit *edit_new(char *str, int x, int y)
{
    struct edit *edit = malloc(sizeof(struct edit));
    edit->buffer = str;
    edit->x = x;
    edit->y = y;
    edit->len = strlen(str);

    return edit;
}  

struct history *history_new() 
{
    struct history *hist = malloc(sizeof(struct history));
    hist->edits   = malloc(sizeof(struct edit*) * H_MAX_LEN);
    hist->len     = 0;
    hist->counter = 0;
    return hist;
}

void history_append_edit(struct editor_state *editor, struct history *hist, char *str, int x, int y)
{
    struct edit *edit = edit_new(str, x, y);
    hist->edits[hist->counter++ % H_MAX_LEN] = edit;
    hist->len = MIN(H_MAX_LEN, hist->len + 1);
}

void history_remove_edit(struct history *hist)
{
    hist->counter = MAX(0, hist->counter - 1);
    hist->len     = MAX(0, hist->len - 1);
}

struct edit *history_get_edit(struct history *hist)
{
    return hist->edits[(hist->counter - 1) % H_MAX_LEN];
}