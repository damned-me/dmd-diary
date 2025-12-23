/*
 * main.c - CLI entry point for dry diary
 */
#include "dry.h"
#include "config.h"
#include "diary.h"

#define usage(T) usages(argv[0], (T))

static void usages(char *name, COMMAND command) {
  switch (command) {
  case LIST:
    fprintf(stderr, "Error: too many arguments!\n");
    printf("Usage: %s list [<today|yesterday|tomorrow|date> [<name>]]\n", name);
    break;
  case SHOW:
    fprintf(stderr, "Error: additional arguments required\n");
    printf("Usage: %s show <id> [<name>]\n", name);
    break;
  case NEW:
    fprintf(stderr, "Error: additional arguments required\n");
    printf("Usage: %s new <video|note> [<name>]\n", name);
    break;
  case INIT:
    fprintf(stderr, "Error, additional arguments required\n");
    printf("Usage: %s init <name> [<path>]\n", name);
    break;
  case DELETE:
    fprintf(stderr, "Error, additional arguments required\n");
    printf("Usage: %s delete <id> <name>\n", name);
    break;
  case HELP:
  default:
    printf("Usage: %s [-v | --version] [-h | --help] <command> [<args>]\n", name);
    printf("\nCOMMANDS\n");
    printf("\tinit <name> [<path>]\tInitialize a new diary\n");
    printf("\tnew <note|video>\tAdd a note or a video with current date-time\n");
    printf("\tshow <date|id>\t\tShow entries for specified date or id\n");
    printf("\tlist <date|date-span>\tList entries for specified date span\n");
    printf("\tdelete <id> <name>\tDelete specified entry\n");
    printf("\texplore [<name>]\tExplore diary with file manager\n");
    break;
  }
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[], char *envp[]) {
  char *dname = NULL;
  char *filter = NULL;
  char *path = NULL;

  config_load();

  if (argc < 2)
    usage(HELP);

  if (strcmp(argv[1], "init") == 0) {
    if (argc < 3) {
      usage(INIT);
    } else {
      if (argc > 3)
        path = argv[3];

      diary_init(argv[2], path);
    }
  } else if (strcmp(argv[1], "new") == 0) {
    if (argc < 3)
      usage(NEW);

    /* Get diary or default */
    if (argc > 3)
      dname = argv[3];

    /* Check type */
    char type = 0;

    if (strcmp(argv[2], "video") == 0)
      type = 'v';
    else if (strcmp(argv[2], "note") == 0)
      type = 'n';

    /* Call new if type is set */
    if (type)
      diary_new(type, dname);
    else
      fprintf(stderr, "Error: wrong type (use 'video' or 'note')\n");

  } else if (strcmp(argv[1], "list") == 0) {
    /* Get diary or default */
    if (argc > 4)
      usage(LIST);

    if (argc > 3)
      dname = argv[3];

    if (argc > 2)
      filter = argv[2];

    diary_list(dname, filter);
  } else if (strcmp(argv[1], "show") == 0) {
    if (argc < 3)
      usage(SHOW);

    if (argc > 3)
      dname = argv[3];

    diary_show(argv[2], dname);
  } else if (strcmp(argv[1], "delete") == 0) {
    if (argc < 4)
      usage(DELETE);

    dname = argv[3];

    diary_delete(argv[2], dname);
  } else if (strcmp(argv[1], "explore") == 0) {
    if (argc > 2)
      dname = argv[2];

    diary_explore(dname);
  } else {
    usage(HELP);
  }

  if (conf != NULL)
    free(conf);

  exit(EXIT_SUCCESS);
}
