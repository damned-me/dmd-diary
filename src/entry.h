/*
 * entry.h - Entry creation and file handling
 */
#ifndef ENTRY_H
#define ENTRY_H

#include "dry.h"

/* Get text entry path for diary */
int get_text_path_by_name(const char *name, char *path);

/* Get video entry path for diary */
int get_video_path_by_name(const char *name, char *path);

/* Create directory tree for current date */
int make_directory_tree(const char *name);

/* Set header in text file */
void set_text_file_header(const char *name, FORMAT fmt);

/* Get command to open text editor */
int get_text_command(const char *name, char *cmd);

/* Get command to record video */
int get_video_command(const char *name, char *cmd);

/* Get file type (TEXT, MEDIA, OTHER) */
FILE_TYPE get_file_type(char *path);

/* Get command to open file based on type */
int open_file_command(char *path, char *cmd);

#endif /* ENTRY_H */
