/*
 * utils.h - Utility functions
 */
#ifndef UTILS_H
#define UTILS_H

#include "dry.h"

/* Expand ~ to home directory in path */
void expand_tilde(const char *input, char *output);

/* Check if file or directory exists */
int do_file_exist(char *path);

/* Format current time into buffer */
size_t get_time(char *buffer, const char *fmt);

#endif /* UTILS_H */
