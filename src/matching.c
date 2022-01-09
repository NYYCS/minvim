#include <stdlib.h>
#include <string.h>

#include "matching.h"
#include "editor.h"
#include "row.h"

#define SIGN(x) (((x) == 0) ? (0) : ((x > 0) ? 1 : -1))


bool match(char *x, char *y)
{   
    for (int i = 0; i < strlen(y); i++) {
        if (i > (strlen(x) - 1) || x[i] != y[i])
            return false;
    }
    return true;
}

struct search_node *search_new(int x, int y)
{
    struct search_node *node = malloc(sizeof(struct search_node));
    node->x = x;
    node->y = y;
    node->prev = NULL;
    node->next = NULL;

    return node;
}

void search_append(struct search_node *node, int x, int y)
{
    struct search_node **next = &node;
    struct search_node *prev = NULL;
    while (*next != NULL) {
        prev = *next;
        next = &(*next)->next;
    }
    *next = search_new(x, y);
    (*next)->prev = prev;
}

struct search_node *scan_for_occurence(struct editor_state *editor, char *pattern)
{
    struct search_node *node = search_new(editor->x, editor->y);

    for (int i = 0; i < editor->row_num; i++) {
        struct row *row = editor->rows[i];
        char *text = row_extract_text(row, true);
        for (int j = 0; j < row->len; j++) {
            if (match(&text[j], pattern)) 
                search_append(node, j + 1, i);
        }
    }
    return node;
}

