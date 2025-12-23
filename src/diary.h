/*
 * diary.h - Diary operations
 */
#ifndef DIARY_H
#define DIARY_H

#include "dry.h"

/* Show flags (bitfield) */
#define SHOW_FLAG_HEAD        0x01  /* Show header with file count before each entry */
#define SHOW_FLAG_INTERLEAVED 0x02  /* Show main entry between each attachment */
#define SHOW_FLAG_TEXT_ONLY   0x04  /* Show only text entries, skip media */
#define SHOW_FLAG_MAIN_ONLY   0x08  /* Show only the main diary entry */

/* Initialize a new encrypted diary */
void diary_init(const char *name, const char *dpath);

/* Create a new entry (note or video) */
void diary_new(char type, const char *name);

/* List diary entries */
void diary_list(const char *name, char *filter);

/* Show entries (by ID or date filter like today/yesterday)
 * flags: combination of SHOW_FLAG_* constants */
void diary_show(char *id_or_filter, const char *name, int flags);

/* Delete a specific entry */
void diary_delete(char *id, const char *name);

/* Explore diary with file manager */
void diary_explore(const char *name);

/* Unlock diary (mount and keep open) */
void diary_unlock(const char *name);

/* Lock diary (unmount) */
void diary_lock(const char *name);

/* Print status of unlocked diaries (for shell prompt integration) */
void diary_status(void);

/* Check if a specific diary is unlocked */
int diary_is_unlocked(const char *name);

#endif /* DIARY_H */
