/*
 * dry.h - Shared types and definitions for dry diary
 */
#ifndef DRY_H
#define DRY_H

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <errno.h>

/* Configuration structure */
typedef struct {
  const char *name;         /* default diary name */
  const char *path;         /* storage directory path */
  const char *editor;       /* text editor command */
  const char *player;       /* video player command */
  const char *list_cmd;     /* directory listing command */
  const char *file_manager; /* file manager/explorer command */
  const char *pager;        /* pager for viewing text files */
} CONFIG;

/* Command types for CLI */
typedef enum { 
  HELP, 
  LIST, 
  SHOW, 
  NEW, 
  INIT, 
  DELETE, 
  EXPLORE 
} COMMAND;

/* Entry format types */
typedef enum { 
  ORG, 
  MARKDOWN, 
  TXT 
} FORMAT;

/* File types for opening */
typedef enum { 
  TEXT, 
  MEDIA, 
  OTHER 
} FILE_TYPE;

#endif /* DRY_H */
