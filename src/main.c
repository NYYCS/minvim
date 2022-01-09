#include <unistd.h>
#include <stdio.h>

#include "editor.h"
#include "buffer.h"
#include "file.h"

int main(int argc, char **argv) {
    struct editor_state *editor = editor_init();
    if (argc == 2)
        pipe_file_to_editor(editor, argv[1]);
        
    while (1) {
        editor_refresh_screen(editor);
        editor_process_key(editor);
    }
}