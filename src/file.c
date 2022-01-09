#include <stdio.h>
#include <ctype.h>

#include "buffer.h"
#include "editor.h"
#include "row.h"

int pipe_file_to_editor(struct editor_state *editor, char *filename)
{
    FILE *fptr = fopen(filename, "r");
    if (fptr == NULL) return -1;
    char c;
    uint i = 0;
    struct row *row = editor->rows[i];

    while ((c = fgetc(fptr)) != EOF) {
        if (c == '\n') {
            if (editor->row_num < i)
                editor_append_row(editor);
            row = editor->rows[++i];
        } else if (!iscntrl(c)) {
            row_insert_character(row, c, row->len);
        }
    }
    
    fclose(fptr);
    return 0;
}

int save_to_file(struct editor_state *editor, char *filename)
{
    FILE *fptr = fopen(filename, "w");
    if (fptr == NULL) return -1;
    struct append_buffer *buf = ab_new();

    for (int i = 0; i < editor->row_num; i++) {
        struct row *row = editor->rows[i];
        ab_append(buf, row_extract_text(row, false), row->len);
        if (i < editor->row_num - 1)
            ab_append(buf, "\n", 1);
    }

    fwrite(buf->buffer, 1, buf->len, fptr);
    fclose(fptr);

    return 0;
}