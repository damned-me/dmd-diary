/*
 * utils.c - Utility functions implementation
 */
#include "utils.h"

void expand_tilde(const char *input, char *output) {
  if (input[0] == '~' && (input[1] == '/' || input[1] == '\0')) {
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    sprintf(output, "%s%s", homedir, input + 1);
  } else {
    strcpy(output, input);
  }
}

int do_file_exist(char *path) {
  struct stat st = {0};
  return !stat(path, &st);
}

size_t get_time(char *buffer, const char *fmt) {
  time_t timer;
  struct tm *tm_info;

  timer = time(NULL);
  tm_info = localtime(&timer);

  return strftime(buffer, 1024, fmt, tm_info);
}
