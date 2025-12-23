/*
 * diary.h - Diary operations
 */
#ifndef DIARY_H
#define DIARY_H

#include "dry.h"

/* Initialize a new encrypted diary */
void diary_init(const char *name, const char *dpath);

/* Create a new entry (note or video) */
void diary_new(char type, const char *name);

/* List diary entries */
void diary_list(const char *name, char *filter);

/* Show a specific entry */
void diary_show(char *id, const char *name);

/* Delete a specific entry */
void diary_delete(char *id, const char *name);

/* Explore diary with file manager */
void diary_explore(const char *name);

#endif /* DIARY_H */
