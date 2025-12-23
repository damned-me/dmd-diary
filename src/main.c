/*
 * main.c - CLI entry point for dry diary
 */
#include "dry.h"
#include "config.h"
#include "diary.h"
#include <getopt.h>

#define VERSION "0.1.0"
#define usage(T) usages(argv[0], (T))

static void print_version(void) {
  printf("dry version %s\n", VERSION);
}

static void print_help(char *name) {
  printf("DRY - Diary and Research-log Keeping Utility\n\n");
  printf("Usage: %s [OPTIONS] <command> [<args>]\n\n", name);
  printf("OPTIONS\n");
  printf("  -h, --help     Show this help message\n");
  printf("  -v, --version  Show version information\n\n");
  printf("COMMANDS\n");
  printf("  init <name> [<path>]     Initialize a new diary\n");
  printf("  new <note|video> [<name>] Add a note or video entry\n");
  printf("  show <id> [<name>]       Show entry by id\n");
  printf("  list [<filter>] [<name>] List entries (today, yesterday, date)\n");
  printf("  delete <id> <name>       Delete an entry\n");
  printf("  explore [<name>]         Open diary in file manager\n");
}

static void print_subcommand_help(COMMAND command) {
  switch (command) {
  case INIT:
    printf("Initialize a new diary\n\n");
    printf("Usage: dry init <name> [<path>]\n\n");
    printf("Arguments:\n");
    printf("  <name>    Name of the diary to create\n");
    printf("  <path>    Optional path where to create the diary\n");
    break;
  case NEW:
    printf("Add a new entry to the diary\n\n");
    printf("Usage: dry new <video|note> [<name>]\n\n");
    printf("Arguments:\n");
    printf("  <type>    Entry type: 'video' or 'note'\n");
    printf("  <name>    Optional diary name (uses default if omitted)\n");
    break;
  case SHOW:
    printf("Show an entry by its ID\n\n");
    printf("Usage: dry show <id> [<name>]\n\n");
    printf("Arguments:\n");
    printf("  <id>      Entry ID to show\n");
    printf("  <name>    Optional diary name (uses default if omitted)\n");
    break;
  case LIST:
    printf("List diary entries\n\n");
    printf("Usage: dry list [<filter>] [<name>]\n\n");
    printf("Arguments:\n");
    printf("  <filter>  Optional filter: 'today', 'yesterday', 'tomorrow', or a date\n");
    printf("  <name>    Optional diary name (uses default if omitted)\n");
    break;
  case DELETE:
    printf("Delete an entry from the diary\n\n");
    printf("Usage: dry delete <id> <name>\n\n");
    printf("Arguments:\n");
    printf("  <id>      Entry ID to delete\n");
    printf("  <name>    Diary name (required)\n");
    break;
  case EXPLORE:
    printf("Open the diary in the file manager\n\n");
    printf("Usage: dry explore [<name>]\n\n");
    printf("Arguments:\n");
    printf("  <name>    Optional diary name (uses default if omitted)\n");
    break;
  case HELP:
  default:
    print_help("dry");
    break;
  }
}

/* Check if any argument is --help or -h */
static int has_help_flag(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
      return 1;
  }
  return 0;
}

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
    print_help(name);
    break;
  }
  exit(command == HELP ? EXIT_SUCCESS : EXIT_FAILURE);
}

int main(int argc, char *argv[], char *envp[]) {
  char *dname = NULL;
  char *filter = NULL;
  char *path = NULL;
  int opt;

  static struct option long_options[] = {
    {"help",    no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}
  };

  /* Parse global options first */
  while ((opt = getopt_long(argc, argv, "+hv", long_options, NULL)) != -1) {
    switch (opt) {
    case 'h':
      print_help(argv[0]);
      exit(EXIT_SUCCESS);
    case 'v':
      print_version();
      exit(EXIT_SUCCESS);
    default:
      print_help(argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  /* Shift argv to point to subcommand */
  argc -= optind;
  argv += optind;

  if (argc < 1) {
    print_help(argv[0] ? argv[0] : "dry");
    exit(EXIT_FAILURE);
  }

  config_load();

  if (strcmp(argv[0], "init") == 0) {
    if (has_help_flag(argc, argv)) {
      print_subcommand_help(INIT);
      exit(EXIT_SUCCESS);
    }
    if (argc < 2) {
      usage(INIT);
    } else {
      if (argc > 2)
        path = argv[2];

      diary_init(argv[1], path);
    }
  } else if (strcmp(argv[0], "new") == 0) {
    if (has_help_flag(argc, argv)) {
      print_subcommand_help(NEW);
      exit(EXIT_SUCCESS);
    }
    if (argc < 2)
      usage(NEW);

    /* Get diary or default */
    if (argc > 2)
      dname = argv[2];

    /* Check type */
    char type = 0;

    if (strcmp(argv[1], "video") == 0)
      type = 'v';
    else if (strcmp(argv[1], "note") == 0)
      type = 'n';

    /* Call new if type is set */
    if (type)
      diary_new(type, dname);
    else
      fprintf(stderr, "Error: wrong type (use 'video' or 'note')\n");

  } else if (strcmp(argv[0], "list") == 0) {
    if (has_help_flag(argc, argv)) {
      print_subcommand_help(LIST);
      exit(EXIT_SUCCESS);
    }
    /* Get diary or default */
    if (argc > 3)
      usage(LIST);

    if (argc > 2)
      dname = argv[2];

    if (argc > 1)
      filter = argv[1];

    diary_list(dname, filter);
  } else if (strcmp(argv[0], "show") == 0) {
    if (has_help_flag(argc, argv)) {
      print_subcommand_help(SHOW);
      exit(EXIT_SUCCESS);
    }
    if (argc < 2)
      usage(SHOW);

    if (argc > 2)
      dname = argv[2];

    diary_show(argv[1], dname);
  } else if (strcmp(argv[0], "delete") == 0) {
    if (has_help_flag(argc, argv)) {
      print_subcommand_help(DELETE);
      exit(EXIT_SUCCESS);
    }
    if (argc < 3)
      usage(DELETE);

    dname = argv[2];

    diary_delete(argv[1], dname);
  } else if (strcmp(argv[0], "explore") == 0) {
    if (has_help_flag(argc, argv)) {
      print_subcommand_help(EXPLORE);
      exit(EXIT_SUCCESS);
    }
    if (argc > 1)
      dname = argv[1];

    diary_explore(dname);
  } else if (strcmp(argv[0], "help") == 0) {
    print_help("dry");
    exit(EXIT_SUCCESS);
  } else {
    fprintf(stderr, "Error: unknown command '%s'\n", argv[0]);
    print_help("dry");
    exit(EXIT_FAILURE);
  }

  if (conf != NULL)
    free(conf);

  exit(EXIT_SUCCESS);
}
