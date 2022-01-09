#ifndef __MATCHING_H__
#define __MATCHING_H__


#include "editor.h"

struct search_node {
    int x, y;
    struct search_node *prev;
    struct search_node *next;
};

struct search_node *search_new(int x, int y);
void search_append(struct search_node *node, int x, int y);

struct search_node *scan_for_occurence(struct editor_state *editor, char *pattern);

#endif
