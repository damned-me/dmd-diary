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
    char video_path[2048];
    char text_path[2048];
    
    get_video_command(name, cmd);
    get_video_path_by_name(name, video_path);
    get_text_path_by_name(name, text_path);

    FILE *fd = fopen(text_path, "a");
    fprintf(fd, "file:%s\n", video_path);
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

/* Helper to check if a string looks like a date filter */
static int is_date_filter(const char *str, char *out_path, size_t out_size) {
  if (strncmp(str, "today", 6) == 0) {
    get_time(out_path, "%Y/%m/%d");
    return 1;
  } else if (strncmp(str, "yesterday", 10) == 0) {
    time_t now = time(NULL);
    struct tm *ts = localtime(&now);
    ts->tm_mday--;
    mktime(ts);
    strftime(out_path, out_size, "%Y/%m/%d", ts);
    return 1;
  } else if (strncmp(str, "tomorrow", 9) == 0) {
    time_t now = time(NULL);
    struct tm *ts = localtime(&now);
    ts->tm_mday++;
    mktime(ts);
    strftime(out_path, out_size, "%Y/%m/%d", ts);
    return 1;
  } else if (strlen(str) == 10 && (str[4] == '-' || str[4] == '/') &&
             (str[7] == '-' || str[7] == '/')) {
    /* Exactly a date (YYYY-MM-DD or YYYY/MM/DD) - must be exactly 10 chars */
    strncpy(out_path, str, out_size - 1);
    out_path[out_size - 1] = '\0';
    /* Convert dashes to slashes */
    for (char *p = out_path; *p; p++) {
      if (*p == '-') *p = '/';
    }
    return 1;
  }
  return 0;
}

/* Helper to extract timestamp from media filename (e.g., "2025-12-23_17-06.mkv" -> "17:06") */
static int extract_time_from_filename(const char *filename, char *time_out, size_t time_size) {
  /* Look for pattern: YYYY-MM-DD_HH-MM in filename */
  const char *p = filename;
  
  /* Find the underscore after date */
  while (*p && *p != '_') p++;
  if (*p != '_') return 0;
  p++; /* Skip underscore */
  
  /* Extract HH-MM and convert to HH:MM */
  if (strlen(p) >= 5 && p[2] == '-') {
    snprintf(time_out, time_size, "%c%c:%c%c", p[0], p[1], p[3], p[4]);
    return 1;
  }
  return 0;
}

/* Helper to print context from a text file for a specific media entry */
static void print_note_context(const char *filepath, const char *media_filename, int max_lines) {
  FILE *f = fopen(filepath, "r");
  if (!f) return;
  
  char time_pattern[16];
  int found_section = 0;
  
  /* Extract time from media filename to find matching section */
  if (!extract_time_from_filename(media_filename, time_pattern, sizeof(time_pattern))) {
    fclose(f);
    return;
  }
  
  char line[512];
  int printed = 0;
  
  printf("  \033[2m"); /* Dim text */
  while (fgets(line, sizeof(line), f)) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
    
    /* Check if this is a level-2 header with our timestamp (e.g., "** 17:06") */
    if (strncmp(line, "** ", 3) == 0 && strstr(line, time_pattern)) {
      found_section = 1;
      printf(" %s\n", line);
      printed++;
      continue;
    }
    
    /* If we found our section, print until next header or max lines */
    if (found_section) {
      /* Stop at next level-1 or level-2 header */
      if (line[0] == '*') break;
      
      /* Skip empty lines */
      char *p = line;
      while (*p == ' ' || *p == '\t') p++;
      if (*p == '\0') continue;
      
      if (printed < max_lines) {
        printf("  %s\n", line);
        printed++;
      }
    }
  }
  printf("\033[0m"); /* Reset */
  
  fclose(f);
}

void diary_show(char *id_or_filter, const char *name, int flags) {
  /*
   * Show diary entries:
   * - If id_or_filter is "today", "yesterday", "tomorrow", or a date: show all entries for that day
   * - Otherwise: treat as an entry ID and show that specific file
   *
   * Flags:
   * - SHOW_FLAG_HEAD: Display header with [n/total] before each file
   * - SHOW_FLAG_INTERLEAVED: Re-show main text entry between each attachment
   *
   * When showing multiple entries, they are displayed sequentially:
   * - Text files: opened in pager (user presses q to continue)
   * - Video/audio: opened in player (output suppressed)
   * - Other files: opened with xdg-open
   */
  char dpath[4096];
  char path[8192];
  char cmd[16384];
  char tme[26];

  if (name == NULL)
    name = get_config()->name;

  if (get_path_by_name(name, dpath)) {
    printf("Error: can't find diary %s\n", name);
    exit(EXIT_FAILURE);
  }

  encdiary(0, name, get_config()->path);

  if (is_date_filter(id_or_filter, tme, sizeof(tme))) {
    /* Show all entries for the date */
    snprintf(path, sizeof(path), "%s/%s", dpath, tme);

    if (!do_file_exist(path)) {
      printf("Error: no entries for '%s' in %s\n", id_or_filter, name);
      encdiary(1, name, get_config()->path);
      exit(EXIT_FAILURE);
    }

    /* Find all files in the directory, sorted by name (chronological order) */
    snprintf(cmd, sizeof(cmd), "find %s -maxdepth 1 -type f 2>/dev/null | sort", path);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
      fprintf(stderr, "Error: failed to list entries\n");
      encdiary(1, name, get_config()->path);
      exit(EXIT_FAILURE);
    }

    /* First pass: collect all files and find main entry */
    char files[256][4096];
    FILE_TYPE ftypes[256];
    int total = 0;
    int main_entry_idx = -1;
    char file_path[4096];

    while (fgets(file_path, sizeof(file_path), fp) != NULL && total < 256) {
      file_path[strcspn(file_path, "\n")] = '\0';
      if (strlen(file_path) > 0) {
        strncpy(files[total], file_path, sizeof(files[total]) - 1);
        files[total][sizeof(files[total]) - 1] = '\0';
        ftypes[total] = get_file_type(file_path);
        
        /* First text file is the main entry */
        if (main_entry_idx < 0 && ftypes[total] == TEXT) {
          main_entry_idx = total;
        }
        total++;
      }
    }
    pclose(fp);

    if (total == 0) {
      printf("No entries found for '%s' in %s\n", id_or_filter, name);
      encdiary(1, name, get_config()->path);
      return;
    }

    /* Show header summary if --head flag (only prints summary, no content) */
    if (flags & SHOW_FLAG_HEAD) {
      printf("=== %s: %d file(s) ===\n", id_or_filter, total);
      for (int i = 0; i < total; i++) {
        const char *fn = strrchr(files[i], '/');
        fn = fn ? fn + 1 : files[i];
        const char *type_str = (ftypes[i] == TEXT) ? "text" :
                               (ftypes[i] == MEDIA) ? "media" : "other";
        printf("  [%d/%d] %s (%s)%s\n", i + 1, total, fn, type_str,
               (i == main_entry_idx) ? " *main*" : "");
      }
      encdiary(1, name, get_config()->path);
      return;
    }

    /* Handle --main flag: show only the main diary entry */
    if ((flags & SHOW_FLAG_MAIN_ONLY) && main_entry_idx >= 0) {
      const char *main_fn = strrchr(files[main_entry_idx], '/');
      main_fn = main_fn ? main_fn + 1 : files[main_entry_idx];
      
      printf("Showing main entry: %s\n", main_fn);
      snprintf(cmd, sizeof(cmd), "%s \"%s\"", get_config()->pager, files[main_entry_idx]);
      system(cmd);
      
      encdiary(1, name, get_config()->path);
      return;
    }

    /* Second pass: display files with appropriate mode */
    int shown = 0;
    for (int i = 0; i < total; i++) {
      /* Skip non-text files if --text flag is set */
      if ((flags & SHOW_FLAG_TEXT_ONLY) && ftypes[i] != TEXT) {
        continue;
      }

      const char *filename = strrchr(files[i], '/');
      filename = filename ? filename + 1 : files[i];

      /* In interleaved mode, show main entry before each non-main entry */
      if ((flags & SHOW_FLAG_INTERLEAVED) && main_entry_idx >= 0 &&
          i != main_entry_idx && ftypes[main_entry_idx] == TEXT) {
        const char *main_fn = strrchr(files[main_entry_idx], '/');
        main_fn = main_fn ? main_fn + 1 : files[main_entry_idx];
        
        printf("\n--- Main entry: %s (before viewing %s) ---\n", main_fn, filename);
        snprintf(cmd, sizeof(cmd), "%s \"%s\"", get_config()->pager, files[main_entry_idx]);
        system(cmd);
      }

      shown++;
      
      switch (ftypes[i]) {
      case TEXT:
        printf("Showing [%d]: %s (text)\n", shown, filename);
        snprintf(cmd, sizeof(cmd), "%s \"%s\"", get_config()->pager, files[i]);
        break;
      case MEDIA:
        printf("Playing [%d]: %s (media)\n", shown, filename);
        /* Show context from main entry if available (section matching this media's timestamp) */
        if (main_entry_idx >= 0 && ftypes[main_entry_idx] == TEXT) {
          print_note_context(files[main_entry_idx], filename, 5);
        }
        /* Suppress ffmpeg/player output by redirecting stderr and stdout to /dev/null */
        snprintf(cmd, sizeof(cmd), "%s \"%s\" >/dev/null 2>&1", get_config()->player, files[i]);
        break;
      case OTHER:
      default:
        printf("Opening [%d]: %s\n", shown, filename);
        snprintf(cmd, sizeof(cmd), "xdg-open \"%s\" >/dev/null 2>&1", files[i]);
        break;
      }
      system(cmd);
    }

    printf("\nShowed %d entry(s) for '%s'\n", total, id_or_filter);
  } else {
    /* Treat as entry ID - parse and find the file */
    char ch[256];
    char *c = ch;
    strncpy(ch, id_or_filter, sizeof(ch) - 1);
    ch[sizeof(ch) - 1] = '\0';
    
    /* Convert date part (- to /) and stop at . or _ */
    while (*c++) {
      if (*c == '-') *c = '/';
      if (*c == '.' || *c == '_') *c = '\0';
    }

    snprintf(path, sizeof(path), "%s/%s/%s", dpath, ch, id_or_filter);

    /* Check if file exists */
    if (!do_file_exist(path)) {
      printf("Error: entry not found %s\n", path);
      encdiary(1, name, get_config()->path);
      exit(EXIT_FAILURE);
    }

    open_file_command(path, cmd);
    system(cmd);
  }

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
