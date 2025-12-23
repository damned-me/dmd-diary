/*
 * entry.c - Entry creation and file handling implementation
 */
#include "entry.h"
#include "config.h"
#include "utils.h"

static char *l1_header_fmt(FORMAT fmt, char *fstring) {
  char *header;
  switch (fmt) {
  case ORG:
    header = "* %Y-%m-%d\n";
    break;
  case MARKDOWN:
    header = "# %Y-%m-%d\n";
    break;
  case TXT:
  default:
    header = "%Y-%m-%d\n";
    break;
  }
  return strcpy(fstring, header);
}

static char *l2_header_fmt(FORMAT fmt, char *fstring) {
  char *header;
  switch (fmt) {
  case ORG:
    header = "** %H:%M:%S\n";
    break;
  case MARKDOWN:
    header = "## %H:%M:%S\n";
    break;
  case TXT:
  default:
    header = "\t%H:%M:%S\n";
    break;
  }
  return strcpy(fstring, header);
}

int get_text_path_by_name(const char *name, char *path) {
  char fmt[128];
  char tmp[2048];
  char *ext = ".org";
  get_path_by_name(name, tmp);

  get_time(fmt, "%%s/%Y/%m/%d/%Y-%m-%d");

  snprintf(path, 2048, fmt, tmp);
  strcat(path, ext);

  return 0;
}

int get_video_path_by_name(const char *name, char *path) {
  char tmp[2048];
  char fmt[128];
  char *ext = ".mkv";
  get_path_by_name(name, tmp);

  get_time(fmt, "%%s/%Y/%m/%d/%Y-%m-%d_%H-%M");

  snprintf(path, 2048, fmt, tmp);
  strcat(path, ext);

  return 0;
}

int make_directory_tree(const char *name) {
  /*
   * Create directory path: /path/to/diary/yyyy/mm/dd/
   */
  char mkdr[4096];
  char subdir[64];
  char path[2048];

  get_path_by_name(name, path);
  get_time(subdir, "%Y/%m/%d");
  snprintf(mkdr, sizeof(mkdr), "mkdir -p %s/%s", path, subdir);
  
  int result = system(mkdr);
  if (result != 0) {
    fprintf(stderr, "Error: failed to create directory tree %s/%s\n", path, subdir);
  }
  return result;
}

void set_text_file_header(const char *name, FORMAT fmt) {
  FILE *fd;
  char path[1024];
  char buffer[1024];
  char fstring[1024];

  get_text_path_by_name(name, path);

  /* if file not exists add level 1 headers */
  if (!do_file_exist(path)) {
    printf("Creating file %s\n", path);

    /* get date header */
    l1_header_fmt(fmt, fstring);
    get_time(buffer, fstring);

    /* create file and write header */
    fd = fopen(path, "w");
    if (fd == NULL) {
      fprintf(stderr, "Error: failed to create file %s\n", path);
      perror("fopen");
      exit(EXIT_FAILURE);
    }
    fprintf(fd, "%s", buffer);
    fclose(fd);
  }

  /* get time header */
  l2_header_fmt(fmt, fstring);
  get_time(buffer, fstring);

  fd = fopen(path, "a");
  if (fd == NULL) {
    fprintf(stderr, "Error: failed to open file %s\n", path);
    perror("fopen");
    exit(EXIT_FAILURE);
  }
  fprintf(fd, "%s", buffer);
  fclose(fd);
}

int get_text_command(const char *name, char *cmd) {
  char path[2048];
  
  if (get_config()->editor == NULL) {
    fprintf(stderr, "Error: text_editor not configured\n");
    fprintf(stderr, "Please set 'text_editor' in your config file\n");
    exit(EXIT_FAILURE);
  }
  
  get_text_path_by_name(name, path);
  return snprintf(cmd, 4096, "%s %s", get_config()->editor, path);
}

int get_video_command(const char *name, char *cmd) {
  char path[2048];
  
  if (get_config()->player == NULL) {
    fprintf(stderr, "Error: video_player not configured\n");
    fprintf(stderr, "Please set 'video_player' in your config file\n");
    exit(EXIT_FAILURE);
  }
  
  /*
   * Webcam recordings (ffmpeg)
   * ffmpeg -f pulse -ac 2 -i default -f v4l2 -i /dev/video0 -t 00:00:20 -vcodec libx264 record.mp4
   */
  char *ffmpeg = "ffmpeg "
    "-f v4l2 "
    "-framerate 30 "
    "-video_size 1024x768 "
    "-input_format mjpeg "
    "-i /dev/video0 "
    "-f pulse "
    "-i default "
    "-ac 1 "
    "-c:a pcm_s16le "
    "-c:v mjpeg "
    "-b:v 64000k "
    "%s "
    "-map 0:v "
    "-vf \"format=yuv420p\" "
    "-f xv display";

  get_video_path_by_name(name, path);
  snprintf(cmd, 4096, ffmpeg, path);

  return 0;
}

FILE_TYPE get_file_type(char *path) {
  FILE_TYPE type;
  char c2[4096];
  char c3[4096];
  char c4[4096];

  /* Check file type */
  snprintf(c2, sizeof(c2), "file %s | grep ASCII > /dev/null", path);
  snprintf(c4, sizeof(c4), "file %s | grep Unicode > /dev/null", path);
  snprintf(c3, sizeof(c3), "file %s | grep Matroska > /dev/null", path);

  if (system(c2) == 0 || system(c4) == 0)
    type = TEXT;
  else if (system(c3) == 0)
    type = MEDIA;
  else
    type = OTHER;

  return type;
}

int open_file_command(char *path, char *cmd) {
  FILE_TYPE type = get_file_type(path);
  if (cmd != NULL) {
    switch (type) {
    case TEXT:
      snprintf(cmd, 4096, "%s %s", get_config()->pager, path);
      break;
    case MEDIA:
      snprintf(cmd, 4096, "%s %s", get_config()->player, path);
      break;
    case OTHER:
      snprintf(cmd, 4096, "xdg-open %s", path);
      break;
    }
  }
  return type;
}
