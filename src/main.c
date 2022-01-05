#include <unistd.h>
#include <stdio.h>

#include "editor.h"
#include "buffer.h"


int main() {
    struct Editor *editor = editor_init();

    while(1) {
        editor_refresh_screen(editor);
        editor_process_key(editor);
    }

}