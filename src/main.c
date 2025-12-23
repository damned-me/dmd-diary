/*
 * main.c - CLI entry point for dry diary
 */
#include "dry.h"
#include "config.h"
#include "diary.h"
#include <getopt.h>

#define VERSION "0.1.0"

static char *prog_name = "dry";  /* Set from argv[0] in main() */

#define usage(T) usages(prog_name, (T))

static void print_version(void) {
  printf("dry version %s\n", VERSION);
}

static void print_help(char *name) {
  printf("DRY - Diary and Research-log Keeping Utility\n\n");
  printf("Usage: %s [OPTIONS] <command> [<args>]\n\n", name);
  printf("OPTIONS\n");
  printf("  -d, --diary <name>  Specify diary to use (default from config)\n");
  printf("  -h, --help          Show this help message\n");
  printf("  -v, --version       Show version information\n\n");
  printf("COMMANDS\n");
  printf("  init <name> [<path>]  Initialize a new diary\n");
  printf("  new <note|video>      Add a note or video entry\n");
  printf("  show <id|filter>      Show entries by ID or date filter\n");
  printf("  list [<filter>]       List entries (today, yesterday, date)\n");
  printf("  delete <id>           Delete an entry\n");
  printf("  explore               Open diary in file manager\n");
  printf("  unlock                Unlock diary for manual editing\n");
  printf("  lock                  Lock diary after manual editing\n");
  printf("  status                Show unlocked diaries (for shell prompt)\n");
}

static void print_subcommand_help(COMMAND command) {
  switch (command) {
  case INIT:
    printf("Initialize a new diary\n\n");
    printf("Usage: %s init <name> [<path>]\n\n", prog_name);
    printf("Arguments:\n");
    printf("  <name>    Name of the diary to create\n");
    printf("  <path>    Optional path where to create the diary\n");
    break;
  case NEW:
    printf("Add a new entry to the diary\n\n");
    printf("Usage: %s [-d <diary>] new <video|note>\n\n", prog_name);
    printf("Arguments:\n");
    printf("  <type>    Entry type: 'video' or 'note'\n\n");
    printf("Options:\n");
    printf("  -d, --diary <name>  Diary to use (default from config)\n");
    break;
  case SHOW:
    printf("Show diary entries\n\n");
    printf("Usage: %s [-d <diary>] show [OPTIONS] <id|filter>\n\n", prog_name);
    printf("Arguments:\n");
    printf("  <id>      Show a specific entry by ID\n");
    printf("  today     Show all entries from today\n");
    printf("  yesterday Show all entries from yesterday\n");
    printf("  <date>    Show all entries from date (YYYY-MM-DD)\n\n");
    printf("Options:\n");
    printf("  -d, --diary <name>  Diary to use (default from config)\n");
    printf("  -m, --main          Show only the main diary entry\n");
    printf("  --text              Show only text entries (skip media)\n");
    printf("  --head              List files only (no content displayed)\n");
    printf("  --interleaved       Re-show main entry before each attachment\n\n");
    printf("When showing multiple entries, they are displayed sequentially:\n");
    printf("  - Text files open in pager (press q to continue)\n");
    printf("  - Videos play in video player\n");
    printf("  - Other files open with default application\n");
    break;
  case LIST:
    printf("List diary entries\n\n");
    printf("Usage: %s [-d <diary>] list [<filter>]\n\n", prog_name);
    printf("Arguments:\n");
    printf("  <filter>  Optional filter: 'today', 'yesterday', 'tomorrow', or a date\n\n");
    printf("Options:\n");
    printf("  -d, --diary <name>  Diary to use (default from config)\n");
    break;
  case DELETE:
    printf("Delete an entry from the diary\n\n");
    printf("Usage: %s [-d <diary>] delete <id>\n\n", prog_name);
    printf("Arguments:\n");
    printf("  <id>      Entry ID to delete\n\n");
    printf("Options:\n");
    printf("  -d, --diary <name>  Diary to use (default from config)\n");
    break;
  case EXPLORE:
    printf("Open the diary in the file manager\n\n");
    printf("Usage: %s [-d <diary>] explore\n\n", prog_name);
    printf("Options:\n");
    printf("  -d, --diary <name>  Diary to use (default from config)\n");
    break;
  case UNLOCK:
    printf("Unlock diary for manual editing\n\n");
    printf("Usage: %s [-d <diary>] unlock\n\n", prog_name);
    printf("Decrypts and mounts the diary, leaving it accessible for manual\n");
    printf("file operations. Remember to lock when done.\n\n");
    printf("Options:\n");
    printf("  -d, --diary <name>  Diary to use (default from config)\n");
    break;
  case LOCK:
    printf("Lock diary after manual editing\n\n");
    printf("Usage: %s [-d <diary>] lock\n\n", prog_name);
    printf("Unmounts and re-encrypts the diary.\n\n");
    printf("Options:\n");
    printf("  -d, --diary <name>  Diary to use (default from config)\n");
    break;
  case STATUS:
    printf("Show unlocked diaries (for shell prompt integration)\n\n");
    printf("Usage: %s status\n\n", prog_name);
    printf("Prints names of currently unlocked diaries, comma-separated.\n");
    printf("Outputs nothing if all diaries are locked.\n\n");
    printf("Shell prompt integration example:\n");
    printf("  # bash\n");
    printf("  PS1='\\$(dry status && echo \" \")\\$ '\n\n");
    printf("  # zsh\n");
    printf("  RPROMPT='\\$(dry status)'\n");
    break;
  case HELP:
  default:
    print_help(prog_name);
    break;
  }
}

static void usages(char *name, COMMAND command) {
  switch (command) {
  case LIST:
    fprintf(stderr, "Error: too many arguments!\n");
    printf("Usage: %s [-d <diary>] list [<today|yesterday|tomorrow|date>]\n", name);
    break;
  case SHOW:
    fprintf(stderr, "Error: additional arguments required\n");
    printf("Usage: %s [-d <diary>] show <id|today|yesterday|date>\n", name);
    break;
  case NEW:
    fprintf(stderr, "Error: additional arguments required\n");
    printf("Usage: %s [-d <diary>] new <video|note>\n", name);
    break;
  case INIT:
    fprintf(stderr, "Error, additional arguments required\n");
    printf("Usage: %s init <name> [<path>]\n", name);
    break;
  case DELETE:
    fprintf(stderr, "Error, additional arguments required\n");
    printf("Usage: %s -d <diary> delete <id>\n", name);
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
  int show_help = 0;
  int show_flags = 0;  /* Flags for show command */

  /* Save program name before any argv manipulation */
  prog_name = argv[0];

  /* Long option codes for options without short equivalents */
  enum {
    OPT_HEAD = 256,
    OPT_INTERLEAVED,
    OPT_TEXT,
    OPT_MAIN
  };

  static struct option long_options[] = {
    {"diary",       required_argument, 0, 'd'},
    {"help",        no_argument,       0, 'h'},
    {"version",     no_argument,       0, 'v'},
    {"head",        no_argument,       0, OPT_HEAD},
    {"interleaved", no_argument,       0, OPT_INTERLEAVED},
    {"text",        no_argument,       0, OPT_TEXT},
    {"main",        no_argument,       0, 'm'},
    {0, 0, 0, 0}
  };

  /* Parse global options first (before subcommand) */
  while ((opt = getopt_long(argc, argv, "+d:hv", long_options, NULL)) != -1) {
    switch (opt) {
    case 'd':
      dname = optarg;
      break;
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
    print_help("dry");
    exit(EXIT_FAILURE);
  }

  /* Save subcommand (don't shift argv - keep it for getopt which needs argv[0]) */
  char *subcmd = argv[0];

  /* Reset getopt fully to enable permutation (finds options anywhere in argv) */
  optind = 0;
  while ((opt = getopt_long(argc, argv, "d:hm", long_options, NULL)) != -1) {
    switch (opt) {
    case 'd':
      dname = optarg;
      break;
    case 'h':
      show_help = 1;
      break;
    case OPT_HEAD:
      show_flags |= SHOW_FLAG_HEAD;
      break;
    case OPT_INTERLEAVED:
      show_flags |= SHOW_FLAG_INTERLEAVED;
      break;
    case OPT_TEXT:
      show_flags |= SHOW_FLAG_TEXT_ONLY;
      break;
    case 'm':
    case OPT_MAIN:
      show_flags |= SHOW_FLAG_MAIN_ONLY;
      break;
    default:
      break;
    }
  }

  /* After getopt, positional args are at argv[optind..argc-1] */
  argv = &argv[optind];
  argc = argc - optind;

  /* Show subcommand help if requested */
  if (show_help) {
    if (strncmp(subcmd, "init", 5) == 0) print_subcommand_help(INIT);
    else if (strncmp(subcmd, "new", 4) == 0) print_subcommand_help(NEW);
    else if (strncmp(subcmd, "list", 5) == 0) print_subcommand_help(LIST);
    else if (strncmp(subcmd, "show", 5) == 0) print_subcommand_help(SHOW);
    else if (strncmp(subcmd, "delete", 7) == 0) print_subcommand_help(DELETE);
    else if (strncmp(subcmd, "explore", 8) == 0) print_subcommand_help(EXPLORE);
    else if (strncmp(subcmd, "unlock", 7) == 0) print_subcommand_help(UNLOCK);
    else if (strncmp(subcmd, "lock", 5) == 0) print_subcommand_help(LOCK);
    else if (strncmp(subcmd, "status", 7) == 0) print_subcommand_help(STATUS);
    else print_help("dry");
    exit(EXIT_SUCCESS);
  }

  config_load();

  if (strncmp(subcmd, "init", 5) == 0) {
    if (argc < 1) {
      usage(INIT);
    } else {
      if (argc > 1)
        path = argv[1];

      diary_init(argv[0], path);
    }
  } else if (strncmp(subcmd, "new", 4) == 0) {
    if (argc < 1)
      usage(NEW);

    /* Check type */
    char type = 0;

    if (strncmp(argv[0], "video", 6) == 0)
      type = 'v';
    else if (strncmp(argv[0], "note", 5) == 0)
      type = 'n';

    /* Call new if type is set */
    if (type)
      diary_new(type, dname);
    else
      fprintf(stderr, "Error: wrong type (use 'video' or 'note')\n");

  } else if (strncmp(subcmd, "list", 5) == 0) {
    if (argc > 1)
      usage(LIST);

    if (argc > 0)
      filter = argv[0];

    diary_list(dname, filter);
  } else if (strncmp(subcmd, "show", 5) == 0) {
    if (argc < 1)
      usage(SHOW);

    diary_show(argv[0], dname, show_flags);
  } else if (strncmp(subcmd, "delete", 7) == 0) {
    if (argc < 1) {
      fprintf(stderr, "Error: delete requires <id>\n");
      usage(DELETE);
    }

    diary_delete(argv[0], dname);
  } else if (strncmp(subcmd, "explore", 8) == 0) {
    diary_explore(dname);
  } else if (strncmp(subcmd, "unlock", 7) == 0) {
    diary_unlock(dname);
  } else if (strncmp(subcmd, "lock", 5) == 0) {
    diary_lock(dname);
  } else if (strncmp(subcmd, "status", 7) == 0) {
    diary_status();
  } else {
    fprintf(stderr, "Error: unknown command '%s'\n", subcmd);
    print_help("dry");
    exit(EXIT_FAILURE);
  }

  if (conf != NULL)
    free(conf);

  exit(EXIT_SUCCESS);
}
