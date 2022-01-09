#ifndef __FILE_H__
#define __FILE_H__

int pipe_file_to_editor(struct editor_state *editor, char *filename);
int save_to_file(struct editor_state *editor, char *filename);

#endif