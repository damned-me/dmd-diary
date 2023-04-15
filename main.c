#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <libconfig.h>

#define usage(T) usages(argv[0], (T))

typedef struct {
  const char *name;
  const char *path;
  const char *editor;
  const char *player;
} CONFIG;

typedef enum { HELP, LIST, SHOW, NEW, INIT, DELETE, EXPLORE } COMMAND;
typedef enum { ORG, MARKDOWN, TXT } FORMAT;

int do_file_exist(char *path) {
  struct stat st = {0};
  return !stat(path, &st);
}

size_t get_time(char *buffer, char *fmt) {
  time_t timer;
  struct tm *tm_info;

  timer = time(NULL);
  tm_info = localtime(&timer);

  return strftime(buffer, 26, fmt, tm_info);
}

void get_conf_path(char *path) {
  // Get home path
  struct passwd *pw = getpwuid(getuid());
  const char *homedir = pw->pw_dir;

  // .dry/dry.conf
  sprintf(path, ".dry/dry.conf");
  if (!do_file_exist(path)) {
    sprintf(path, "%s/.dry/dry.conf", homedir);
    // ~/.dry/dry.conf
    if (!do_file_exist(path)) {
      sprintf(path, "/etc/%s", "dry.conf");
      // /etc/dry.conf
      if (!do_file_exist(path)) {
        fprintf(stderr, "Error: can't find config file");
        fprintf(stderr,
                "Create config file in\n~/.dry/dry.conf\n/etc/dry.conf");
        exit(1);
      }
    }
  }
}

void get_ref_path(char *path) {
  // Get home path
  struct passwd *pw = getpwuid(getuid());
  const char *homedir = pw->pw_dir;

  // .dry/dry.conf
  sprintf(path, ".dry/diaries.ref");
  if (!do_file_exist(path)) {
    sprintf(path, "%s/.dry/diaries.ref", homedir);
    // ~/.dry/dry.conf
    if (!do_file_exist(path)) {
      fprintf(stderr, "Error: can't find config file");
      fprintf(stderr, "Create config file in\n~/.dry/.conf\n/etc/dry.conf");
      exit(1);
    }
  }
}

CONFIG *conf = NULL;
CONFIG *get_config() {
  /* char path[1024], name[1024], value[1024]; */

  /* if (conf == NULL) */
  /*   conf = malloc(sizeof(CONFIG)); */

  /* get_conf_path(path); */

  /* FILE *fd = fopen(path, "r"); */
  /* while (fscanf(fd, "%s = %[^\n]%*c", name, value) == 2) { */
  /*   // printf("%s:%s\n", name, value); */
  /*   size_t len = strlen(value); */
  /*   if (strcmp(name, "default_diary") == 0) { */
  /*     conf->name = calloc(len, sizeof(char)); */
  /*     strncpy(conf->name, value, len); */
  /*   } else if (strcmp(name, "text_editor") == 0) { */
  /*     conf->editor = calloc(len, sizeof(char)); */
  /*     strncpy(conf->editor, value, len); */
  /*   } else if (strcmp(name, "video_player") == 0) { */
  /*     conf->player = calloc(len, sizeof(char)); */
  /*     strncpy(conf->player, value, len); */
  /*   } else if (strcmp(name, "default_dir") == 0) { */
  /*     conf->path = calloc(len, sizeof(char)); */
  /*     strncpy(conf->path, value, len); */
  /*   } */
  /* } */
  return conf;
}
int config() {
  config_t cfg;
  config_setting_t *setting;
  const char *str;
  conf = (CONFIG *) malloc(sizeof(CONFIG));
  config_init(&cfg);

  char path[1024];
  get_conf_path(path);

  if(!config_read_file(&cfg, path)) {
    fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
            config_error_line(&cfg), config_error_text(&cfg));
    config_destroy(&cfg);
    return (EXIT_FAILURE);
  }

  if(!config_lookup_string(&cfg, "default_diary", &conf->name))
    fprintf(stderr, "No 'default_diary' setting in configuration file.\n");
  if (!config_lookup_string(&cfg, "text_editor", &conf->editor))
    fprintf(stderr, "No 'text_editor' setting in configuration file.\n");
  if(!config_lookup_string(&cfg, "video_player", &conf->player))
    fprintf(stderr, "No 'video_player' setting in configuration file.\n");
  if(!config_lookup_string(&cfg, "default_dir", &conf->path))
    fprintf(stderr, "No 'default_dir' setting in configuration file.\n");

  return(EXIT_SUCCESS);
}
int get_path_by_name(const char *dname, char *path) {
  FILE *fd;
  char rpath[1024];
  char name[512];
  char value[1024];

  get_ref_path(rpath);

  fd = fopen(rpath, "r");
  while (fscanf(fd, "%s : %s", name, value) > 0) {
    if (strcmp(name, dname) == 0) {
      strcpy(path, value);
      fclose(fd);
      return 0;
    }
  }
  fclose(fd);
  return 1;
}
void usages(char *name, COMMAND command) {
  switch(command) {
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
  case HELP:
  default:
    printf("Usage: %s [-v | --version] [-h | --help] <command> [<args>]\n", name);
    printf("\nCOMMANDS\n");
    printf("\tinit <name> [<path>]\tInitialize a new diary\n");
    printf("\tnew <note|video>\tAdd a note or a video with current date-time\n");
    printf("\tshow <date|id>\t\tShow entries for specified date or id\n");
    printf("\tlist <date|date-span>\tList entries for specified date span\n");
    printf("\tdelete <id> <name>\tDelete specified entry\n");
    break;
  }
  exit(1);
}
/*
Encryption (encfs)
https://www.baeldung.com/linux/encrypting-decrypting-directory
https://linux.die.net/man/1/encfs

Create                      mode                        /encr            /decr
$ echo "password" | encfs --paranoia --stdinpass ~/.dry/storage/%s ~/.dry/storage/.%s

Open
$ echo "password" | encfs --stdinpass ~/.dry/storage/%s ~/.dry/storage/.%s

Close
$ fusermount -u ~/.dry/storage/%s

IDEAS
- Build a buckets hash list using directory of encfs
  Save reference as hash (id)
*/

void encdiary(int opcl, const char *name, const char *path){
  char *fcmd;
  char cmd[2048];
  if(name == NULL)
    name = get_config()->name;

  if(path == NULL)
    path = get_config()->path;

  if (!opcl) {
    char password[1024];
    fcmd =
        "encfs %s/.%s %s/%s";
    sprintf(cmd, fcmd, path, name, path, name);
  }
  else {
    fcmd =
        "fusermount -u %s/%s";
    sprintf(cmd, fcmd, path, name);
    /* puts(cmd); */
  }
  system(cmd);
}


void init(const char *name, const char *dpath) {
  /*
    1. create dir
    2. create reference to diary
    3. create dirtree
    4. create encrypted fs

    .dry/
    storage/      # storage dir
    diaries.ref   # ref file
    dry.conf      # config file
  */
  char fref[1024];
  char path[1024];
  char cmd[1024];

  if(dpath == NULL)
    dpath = get_config()->path;

  /* check if already exist */
  if (!get_path_by_name(name, path)) {
    printf("Diary already exist at %s\n", path);
    exit(1);
  }

  sprintf(path, "%s/%s", dpath, name);

  /* create enc fs if not exist */
  /* if (!do_file_exist(path)){ */
    //mkdir(path, 0700);
  sprintf(cmd, "encfs --paranoia %s/.%s %s/%s", dpath, name, dpath, name);
  system(cmd);
  /* } */
  /* else { */
  /*   char c; */
  /*   printf("Directory already exist at %s and it may not be empty\n" */
  /*          "Would you like to add a reference to it? [y/N]\n", */
  /*          path); */
  /*   scanf("%c", &c); */
  /*   if (c != 'y' && c != 'Y') */
  /*     exit(1); */
  /* } */

  /* add reference to diary to ref file */
  get_ref_path(fref);
  FILE *fd = fopen(fref, "a");
  fprintf(fd, "%s : %s\n", name, path);

  printf("Created new diary %s at %s\n", name, path);

  encdiary(1, name, dpath);
}

void newe(char type, const char *name) {
  /*
    1. find diary
    2. decrypt diary
    3. create file
    4. encrypt diary
  */
  // filename format: YYYY-MM-DD.ext
  char buffer[26];
  char file_path[1024];
  char file_name[26];
  char cmd[1024];
  char command[1024];
  char path[1024];
  char mkdr[1024];
  char tmp[1024];

  FILE *fd;
  FORMAT fmt = ORG; // TODO: possibile formats: markdown, org

  // Find diary or default if null
  // set default diary name
  if(name == NULL)
    name = get_config()->name;

  if (get_path_by_name(name, path) != 0) {
    printf("Error: can't find diary %s\n", name);
    exit(1);
   };

  encdiary(0, name, get_config()->path);
  // Get current date-time
  // TODO decrypt diary

  // TODO path to write

  // TODO  mkdir dirtree
  get_time(file_name, "%Y/%m/%d");
  get_path_by_name(name, tmp);
  sprintf(mkdr, "mkdir -p %s/%s", tmp, file_name);
  system(mkdr);

  get_time(tmp, "%Y-%m-%d");

  sprintf(file_path, "%s/%s/%s", path, file_name, tmp);

  // create file
  printf("Creating new %s\n", type == 'v' ? "video" : "note");
  if(type == 'v') {
    sprintf(file_path, "%s/%s/%s.org", path, file_name, tmp);

    // Check if file exists and add date time as header
    if (!do_file_exist(file_path)) {
      printf("Creating file %s\n", file_path);

      fd = fopen(file_path, "w");

      // Print to file
      if (fmt == ORG) {
	get_time(buffer, "%Y-%m-%d");
        fprintf(fd, "* %s\n", buffer);
      }
      fclose(fd);
    }

    fd = fopen(file_path, "a");
    get_time(buffer, "%H:%M:%S");
    sprintf(file_path, "%s/%s/%s", path, file_name, tmp);

    // Print to file
    if(fmt == ORG){
      get_time(tmp, "%Y-%m-%d_%H-%M.mkv");
      fprintf(fd, "** %s\nfile:%s\n", buffer, tmp);
    }
    fclose(fd);

    get_time(file_name, "_%H-%M");
    strcat(file_path, file_name);
    /*
      Webcam recordings (ffmpeg)
      https://askubuntu.com/questions/1445157/how-to-record-webcam-with-audio-on-ubuntu-22-04-from-cli
      https://ffmpeg.org/

      ffmpeg -f pulse -ac 2 -i default -f v4l2 -i /dev/video0 -t 00:00:20 -vcodec libx264 record.mp4
      The -f defines the format, pulse for audio and v4l2(Video 4 Linux 2).
      The -ac defines the audio channels, in this case 2.
      The -i defines the input, default for audio and the webcam for video.
      The -t defines the duration of the recording, in this case 20 seconds.
      The -vcodec defines the output video codec, since it is an mp4 file it is set to libx264 (H.264)
      Audio should default to AAC so -acodec is not needed.
    */
    char* ffmpeg = "ffmpeg -f v4l2 \
    -framerate 30 \
    -video_size 1024x768 \
    -input_format mjpeg \
    -i /dev/video0 \
    -f pulse \
    -ac 1\
    -i default \
    -c:a pcm_s16le \
    -c:v mjpeg \
    -b:v 64000k \
    %s.mkv \
    -map 0:v \
    -vf \"format=yuv420p\" \
    -f xv display";
    strcpy(command, ffmpeg);
    // strcpy(command, "ffmpeg -threads 125 -f pulse -ac 1 -i default -thread_queue_size 32 -input_format mjpeg -i /dev/video0 -f mjpeg - %s.mkv 2>/dev/null | ffplay - 2>/dev/null");

  }
  if(type == 'n') {
    strcat(file_path, ".org");

    // Check if file exists and add date time as header
    if (!do_file_exist(file_path)) {
      get_time(buffer, "%Y-%m-%d");
      printf("Creating file %s\n", file_path);

      fd = fopen(file_path, "w");

      // Print to file
      switch (fmt) {
      case ORG:
        fprintf(fd, "* %s\n", buffer);
        break;
      case MARKDOWN:
        fprintf(fd, "# %s\n", buffer);
        break;
      case TXT:
      default:
        fprintf(fd, "%s\n", buffer);
        break;
      }

      fclose(fd);
    }

    fd = fopen(file_path, "a");
    get_time(buffer, "%H:%M:%S");

    // Print to file
    switch (fmt) {
    case ORG:
      fprintf(fd, "** %s\n", buffer);
      break;
    case MARKDOWN:
      fprintf(fd, "## %s\n", buffer);
      break;
    case TXT:
    default:
      fprintf(fd, "\t%s\n", buffer);
      break;
    }
    fclose(fd);

    /* call file and editor */
    sprintf(command, "%s %%s", get_config()->editor);
  }

  sprintf(cmd, command, file_path);

  //Execute
  system(cmd);
  printf("Written %s\n", "output");
  // encrypt diary

  encdiary(1, name, get_config()->path);
  // TODO
}

void list(const char *name, char *filter) {
  char cmd[1024];
  char dpath[1024];
  char path[1024];

  char *c = "exa -hal %s";

  if (name == NULL)
    name = get_config()->name;

  if (get_path_by_name(name, dpath)) {
    printf("Error: can't find diary %s\n", name);
    exit(0);
  }

  encdiary(0, name, get_config()->path);

  char tme[26];
  get_time(tme, "%Y/%m/%d");

  if(filter != NULL && strcmp(filter, "yesterday") == 0){
    time_t now;
    struct tm *ts;

    now = time(NULL);
    ts = localtime(&now);
    ts->tm_mday--;
    mktime(ts); /* Normalise ts */
    strftime(tme, sizeof(tme), "%Y/%m/%d", ts);
  }

  sprintf(path, "%s/%s", dpath, tme);

  // Check if file exists
  if (!do_file_exist(path)){
    printf("Error: no entry for %s in %s\n", filter, path);

    encdiary(1, name, get_config()->path);
    exit(1);
  }

  sprintf(cmd, c, path);

  system(cmd);

  encdiary(1, name, get_config()->path);
}

void show(char *id, const char *name) {
  char dpath[1024];
  char path[1024];
  char cmd[1024];
  char c2[1024];
  char c3[1024];
  char c4[1024];

  if(name == NULL)
    name = get_config()->name;

  if(get_path_by_name(name, dpath)) {
    printf("Error: can't find diary %s", name);
    exit(1);
  };

  encdiary(0, name, get_config()->path);
  //char time[26];
  // get_time(time, "%Y/%m/%d");

  /* TODO: THIS CODE SUCKS */
  char ch[1024];
  char *c = ch;
  strncpy(ch, id, 1024);
  while(*c++) {
    if(*c == '-') *c = '/';
    if(*c == '.' || *c == '_') *c = '\0';
  }
  /* ----- */

  sprintf(path, "%s/%s/%s", dpath, ch, id);
  // Check if file exists
  if (!do_file_exist(path)){
    printf("Error: file not found %s\n", path);
    encdiary(1, name, get_config()->path);
    exit(1);
  }

  // Check file type
  sprintf(c2, "file %s | grep ASCII > /dev/null", path);
  sprintf(c4, "file %s | grep Unicode > /dev/null", path);
  sprintf(c3, "file %s | grep Matroska > /dev/null", path);

  if (system(c2) == 0 || system(c4) == 0)
    sprintf(cmd, "less %s", path);
  else if (system(c3) == 0)
    sprintf(cmd, "%s %s", get_config()->player, path);
  else
    sprintf(cmd, "xdg-open %s", path);

  printf(cmd);
  // Display file
  system(cmd);
  encdiary(1, name, get_config()->path);
}

void delete(char *id, const char *name){
  char dpath[1024];
  char path[1024];
  char cmd[1024];
  char c2[1024];
  char c3[1024];
  char c4[1024];
  char ch[1024];

  if(name == NULL)
    name = get_config()->name;

  if(get_path_by_name(name, dpath)) {
    printf("Error: can't find diary %s", name);
    exit(1);
  };
  encdiary(0, name, get_config()->path);
  //char time[26];
  // get_time(time, "%Y/%m/%d");

  /* TODO: THIS CODE SUCKS */
  char *c = ch;
  strncpy(ch, id, 1024);
  while(*c++) {
    if(*c == '-') *c = '/';
    if(*c == '.' || *c == '_') *c = '\0';
  }
  /* ----- */

  sprintf(path, "%s/%s/%s", dpath, ch, id);
  // Check if file exists
  if (!do_file_exist(path)){
    printf("Error: file not found %s\n", path);
    encdiary(1, name, get_config()->path);
    exit(1);
  }

  printf("Deleting %s\n", path);

  sprintf(cmd, "rm %s", path);

  // Display file
  system(cmd);
  encdiary(1, name, get_config()->path);
}

void explore(const char *name){
  char path[1024];
  char cmd[1024];
  char c2[1024];
  char c3[1024];
  char c4[1024];
  char ch[1024];

  if(name == NULL)
    name = get_config()->name;

  if(get_path_by_name(name, path)) {
    printf("Error: can't find diary %s", name);
    exit(1);
  };
  encdiary(0, name, get_config()->path);


  // Check if file exists
  if (!do_file_exist(path)){
    printf("Error: dir not found %s\n", path);
    encdiary(1, name, get_config()->path);
    exit(1);
  }

  sprintf(cmd, "ranger %s", path);

  // Display files
  system(cmd);
  encdiary(1, name, get_config()->path);
}

int main(int argc, char *argv[], char *envp[]) {
  char *dname = NULL;
  char *filter = NULL;
  char *path = NULL;

  config();

  if (argc < 2)
    usage(HELP);

  if (strcmp(argv[1], "init") == 0) {
    if (argc < 3) {
      usage(INIT);
    } else {
      if (argc > 3)
        path = argv[3];

      init(argv[2], path);
    }
  } else if (strcmp(argv[1], "new") == 0) {
    if (argc < 3)
      usage(NEW);

    // Get diary or default
    if (argc > 3)
      dname = argv[3];

    // Check type
    char type = 0;

    if (strcmp(argv[2], "video") == 0)
      type = 'v';
    else if (strcmp(argv[2], "note") == 0)
      type = 'n';

    // Call new if type is set
    if (type)
      newe(type, dname);
    else
      perror("Error: wrong type\n");

  } else if (strcmp(argv[1], "list") == 0) {
    // Get diary or default
    if (argc > 4)
      usage(LIST);

    if (argc > 3)
      dname = argv[3];

    if (argc > 2)
      filter = argv[2];

    list(dname, filter);
  } else if (strcmp(argv[1], "show") == 0) {
    if (argc < 3)
      usage(SHOW);

    if(argc > 3)
      dname = argv[3];

    show(argv[2], dname);
  } else if (strcmp(argv[1], "delete") == 0) {
    if (argc < 4)
      usage(DELETE);

    dname = argv[3];

    delete(argv[2], dname);
  } else if (strcmp(argv[1], "explore") == 0) {
    if (argc > 2)
      dname = argv[2];

    explore(dname);
  }
  if (conf != NULL)
    free(conf);

  exit(EXIT_SUCCESS);
}
