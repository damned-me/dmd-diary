/*
 * diary.c - Diary operations implementation
 */
#include "diary.h"
#include "config.h"
#include "crypto.h"
#include "entry.h"
#include "utils.h"

void diary_init(const char *name, const char *dpath) {
  /*
   * Initialize a new encrypted diary:
   * 1. Create encrypted source directory
   * 2. Create mount point
   * 3. Initialize encfs
   * 4. Add reference to diaries.ref
   */
  char fref[2048];
  char path[2048];
  char enc_path[2048];
  char cmd[8192];
  
  if (dpath == NULL)
    dpath = get_config()->path;

  /* check if already exist */
  if (!get_path_by_name(name, path)) {
    printf("Diary already exists at %s\n", path);
    exit(EXIT_FAILURE);
  }

  snprintf(path, sizeof(path), "%s/%s", dpath, name);
  snprintf(enc_path, sizeof(enc_path), "%s/.%s", dpath, name);
  
  /* create parent storage directory if needed */
  snprintf(cmd, sizeof(cmd), "mkdir -p -m 0700 %s", dpath);
  if (system(cmd) != 0) {
    fprintf(stderr, "Error: failed to create storage directory %s\n", dpath);
    exit(EXIT_FAILURE);
  }
  
  /* create encrypted source directory */
  if (mkdir(enc_path, 0700) != 0 && errno != EEXIST) {
    fprintf(stderr, "Error: failed to create directory %s: %s\n", enc_path, strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  /* create mount point */
  if (mkdir(path, 0700) != 0 && errno != EEXIST) {
    fprintf(stderr, "Error: failed to create directory %s: %s\n", path, strerror(errno));
    exit(EXIT_FAILURE);
  }

  /* create encrypted filesystem */
  snprintf(cmd, sizeof(cmd), "encfs --paranoia %s %s", enc_path, path);
  if (system(cmd) != 0) {
    fprintf(stderr, "Error: failed to create encrypted filesystem\n");
    /* cleanup on failure */
    rmdir(path);
    rmdir(enc_path);
    exit(EXIT_FAILURE);
  }

  /* add reference to diary to ref file */
  get_ref_path(fref);
  FILE *fd = fopen(fref, "a");
  if (fd == NULL) {
    fprintf(stderr, "Error: failed to open reference file %s\n", fref);
    exit(EXIT_FAILURE);
  }
  fprintf(fd, "%s : %s\n", name, path);
  fclose(fd);

  printf("Created new diary %s at %s\n", name, path);

  /* unmount after initialization */
  encdiary(1, name, dpath);
}

void diary_new(char type, const char *name) {
  FORMAT fmt = ORG;
  char cmd[1024];
  char path[1024];

  if (name == NULL)
    name = get_config()->name;

  /* check if diary exists */
  if (get_path_by_name(name, path) != 0) {
    printf("Error: can't find diary %s\n", name);
    exit(EXIT_FAILURE);
  }

  /* decrypt diary */
  encdiary(0, name, get_config()->path);

  /* create directory tree */
  make_directory_tree(name);

  /* create entry */
  printf("Creating new %s\n", type == 'v' ? "video" : "note");

  set_text_file_header(name, fmt);

  if (type == 'v') {
    get_video_command(name, cmd);

    get_text_path_by_name(name, path);

    FILE *fd = fopen(path, "a");
    fprintf(fd, "file:%s\n", path);
    fclose(fd);
  }
  else if (type == 'n') {
    get_text_command(name, cmd);
  }

  /* Execute */
  system(cmd);
  printf("Written %s\n", "output");

  /* encrypt diary */
  encdiary(1, name, get_config()->path);
}

void diary_list(const char *name, char *filter) {
  char cmd[16384];
  char dpath[4096];
  char path[8192];

  const char *list_cmd = get_config()->list_cmd;

  if (name == NULL)
    name = get_config()->name;

  if (get_path_by_name(name, dpath)) {
    printf("Error: can't find diary %s\n", name);
    exit(EXIT_SUCCESS);
  }

  encdiary(0, name, get_config()->path);

  char tme[26];
  int use_filter_path = 1;

  if (filter == NULL) {
    /* No filter: list entire diary */
    use_filter_path = 0;
  } else if (strncmp(filter, "today", 6) == 0) {
    get_time(tme, "%Y/%m/%d");
  } else if (strncmp(filter, "yesterday", 10) == 0) {
    time_t now = time(NULL);
    struct tm *ts = localtime(&now);
    ts->tm_mday--;
    mktime(ts);
    strftime(tme, sizeof(tme), "%Y/%m/%d", ts);
  } else if (strncmp(filter, "tomorrow", 9) == 0) {
    time_t now = time(NULL);
    struct tm *ts = localtime(&now);
    ts->tm_mday++;
    mktime(ts);
    strftime(tme, sizeof(tme), "%Y/%m/%d", ts);
  } else {
    /* Assume filter is a date in YYYY/MM/DD or YYYY-MM-DD format */
    strncpy(tme, filter, sizeof(tme) - 1);
    tme[sizeof(tme) - 1] = '\0';
    /* Convert dashes to slashes if present */
    for (char *p = tme; *p; p++) {
      if (*p == '-') *p = '/';
    }
  }

  if (use_filter_path) {
    snprintf(path, sizeof(path), "%s/%s", dpath, tme);

    /* Check if path exists */
    if (!do_file_exist(path)) {
      printf("Error: no entries for '%s' in %s\n", filter, name);
      encdiary(1, name, get_config()->path);
      exit(EXIT_FAILURE);
    }
  } else {
    /* List entire diary */
    strcpy(path, dpath);
  }

  snprintf(cmd, sizeof(cmd), "%s %s", list_cmd, path);

  system(cmd);

  encdiary(1, name, get_config()->path);
}

void diary_show(char *id, const char *name) {
  char dpath[4096];
  char path[8192];
  char cmd[8192];

  if (name == NULL)
    name = get_config()->name;

  if (get_path_by_name(name, dpath)) {
    printf("Error: can't find diary %s", name);
    exit(EXIT_FAILURE);
  }

  encdiary(0, name, get_config()->path);

  /* Parse id to path (convert - to /, stop at . or _) */
  char ch[256];
  char *c = ch;
  strncpy(ch, id, sizeof(ch) - 1);
  ch[sizeof(ch) - 1] = '\0';
  while (*c++) {
    if (*c == '-') *c = '/';
    if (*c == '.' || *c == '_') *c = '\0';
  }

  snprintf(path, sizeof(path), "%s/%s/%s", dpath, ch, id);

  /* Check if file exists */
  if (!do_file_exist(path)) {
    printf("Error: file not found %s\n", path);
    encdiary(1, name, get_config()->path);
    exit(EXIT_FAILURE);
  }

  open_file_command(path, cmd);

  /* Display file */
  system(cmd);
  encdiary(1, name, get_config()->path);
}

void diary_delete(char *id, const char *name) {
  char dpath[4096];
  char path[8192];
  char cmd[16384];
  char ch[256];

  if (name == NULL)
    name = get_config()->name;

  if (get_path_by_name(name, dpath)) {
    printf("Error: can't find diary %s", name);
    exit(EXIT_FAILURE);
  }

  encdiary(0, name, get_config()->path);

  /* Parse id to path */
  char *c = ch;
  strncpy(ch, id, sizeof(ch) - 1);
  ch[sizeof(ch) - 1] = '\0';
  while (*c++) {
    if (*c == '-') *c = '/';
    if (*c == '.' || *c == '_') *c = '\0';
  }

  snprintf(path, sizeof(path), "%s/%s/%s", dpath, ch, id);

  /* Check if file exists */
  if (!do_file_exist(path)) {
    printf("Error: file not found %s\n", path);
    encdiary(1, name, get_config()->path);
    exit(EXIT_FAILURE);
  }

  printf("Deleting %s\n", path);

  snprintf(cmd, sizeof(cmd), "%s %s", get_config()->file_manager, path);

  system(cmd);
  encdiary(1, name, get_config()->path);
}

void diary_explore(const char *name) {
  char path[4096];
  char cmd[8192];

  if (name == NULL)
    name = get_config()->name;

  if (get_path_by_name(name, path)) {
    printf("Error: can't find diary %s", name);
    exit(EXIT_FAILURE);
  }

  encdiary(0, name, get_config()->path);

  /* Check if file exists */
  if (!do_file_exist(path)) {
    printf("Error: dir not found %s\n", path);
    encdiary(1, name, get_config()->path);
    exit(EXIT_FAILURE);
  }

  snprintf(cmd, sizeof(cmd), "%s %s", get_config()->file_manager, path);

  /* Display files */
  system(cmd);
  encdiary(1, name, get_config()->path);
}
