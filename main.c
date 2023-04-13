#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>

typedef struct {
  char *name;
  char *path;
  char *editor;
  char *player;
} CONFIG;

typedef enum { HELP, LIST, SHOW, NEW, INIT, DELETE } COMMAND;
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
  char path[1024], name[1024], value[1024];

  if (conf == NULL)
    conf = malloc(sizeof(CONFIG));

  get_conf_path(path);

  FILE *fd = fopen(path, "r");
  while (fscanf(fd, "%s = %[^\n]%*c", name, value) == 2) {
    // printf("%s:%s\n", name, value);
    size_t len = strlen(value);
    if (strcmp(name, "default_diary") == 0) {
      conf->name = calloc(len, sizeof(char));
      strncpy(conf->name, value, len);
    } else if (strcmp(name, "text_editor") == 0) {
      conf->editor = calloc(len, sizeof(char));
      strncpy(conf->editor, value, len);
    } else if (strcmp(name, "video_player") == 0) {
      conf->player = calloc(len, sizeof(char));
      strncpy(conf->player, value, len);
    } else if (strcmp(name, "default_dir") == 0) {
      conf->path = calloc(len, sizeof(char));
      strncpy(conf->path, value, len);
    }
  }
  return conf;
}

int get_path_by_name(char *dname, char *path) {
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

/*
Encryption (encfs)
https://www.baeldung.com/linux/encrypting-decrypting-directory
https://linux.die.net/man/1/encfs

Create
$ echo "password" | encfs --standard --stdinpass ~/encrypted
~/unencrypted

Open
$ echo "password" | encfs --sdinpass ~/encrypted ~/unencrypted

Close
$ fusermount -u ~/unencrypted

IDEAS
- Build a buckets hash list using directory of encfs
  Save reference as hash (id)
*/
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

void init(char *name, char *dpath) {
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

  if(dpath == NULL)
    dpath = get_config()->path;

  // check if already exist
  if (!get_path_by_name(name, path)) {
    printf("Diary already exist at %s\n", path);
    exit(1);
  }

  sprintf(path, "%s/%s", dpath, name);

  // create dir if not exist
  if (!do_file_exist(path))
    mkdir(path, 0700);
  else {
    char c;
    printf("Diary dir already exist at %s\nWould you like to add a reference to it? [y/N]\n", path);
    scanf("%c", &c);
    if(c != 'y' && c != 'Y')
      exit(1);
  }

  // add reference to diary to ref file
  get_ref_path(fref);
  FILE *fd = fopen(fref, "a");
  fprintf(fd, "%s : %s\n", name, path);

  printf("Created new diary %s at %s\n", name, path);
}

void newe(char type, char *name) {
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
  FILE *fd;
  FORMAT fmt = ORG; // TODO: possibile formats: markdown, org

  // Find diary or default if null
  // set default diary name
  if(name == NULL)
    name = get_config()->name;

  if(get_path_by_name(name, path) != 0) {
    printf("Error: can't find diary %s\n", name);
    exit(1);
  };

  // Get current date-time
  // TODO decrypt diary

  // TODO path to write

  // TODO  mkdir dirtree
  char mkdr[1024];
  char tmp[1024];
  get_time(file_name, "%Y/%m/%d");
  get_path_by_name(name, tmp);
  sprintf(mkdr, "mkdir -p %s/%s", tmp, file_name);
  system(mkdr);

  get_time(tmp, "%Y-%m-%d");

  sprintf(file_path, "%s/%s/%s", path, file_name, tmp);
  // create file
  printf("Creating new %s\n", type == 'v' ? "video" : "note");
  if(type == 'v') {
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
  // TODO
}

void list(char *name, char *filter) {
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
    exit(1);
  }

  sprintf(cmd, c, path);

  system(cmd);
}

void show(char *id, char *name) {
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
}

void delete(char *id, char *name){
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
    exit(1);
  }

  printf("Deleting %s\n", path);

  sprintf(cmd, "rm %s", path);

  // Display file
  system(cmd);
}
#define usage(T) usages(argv[0], (T))

int main(int argc, char *argv[], char *envp[]) {
  char *dname = NULL;
  char *filter = NULL;
  char *path = NULL;
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
  }
  if (conf != NULL)
    free(conf);

  exit(0);
}
