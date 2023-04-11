#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wait.h>
#include <time.h>

#define HOMEPATH ".dry/storage"
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
typedef enum { HELP, LIST, SHOW, NEW, INIT } COMMAND;
void usages(char *name, COMMAND command) {
  switch(command) {
  case LIST:
    printf("Usage: %s list [<today|yesterday|tomorrow|date> [<name>]]\n", name);
    fprintf(stderr, "Error: too many arguments!");
    break;
  case SHOW:
    printf("Usage: %s show <id> [<name>]\n", name);
    fprintf(stderr, "Error: additional arguments required\n");
    break;
  case NEW:
    printf("Usage: %s new <video|note> [<name>]\n", name);
    fprintf(stderr, "Error: additional arguments required\n");
    break;
  case INIT:
    printf("Usage: %s init <name> [<path>]\n", name);
    fprintf(stderr, "Error, additional arguments required\n");
    break;
  case HELP:
  default:
    printf("Usage: %s [-v | --version] [-h | --help] <command> [<args>]\n", name);
    printf("\nCOMMANDS\n");
    printf("\tinit\t\t\tInitialize a new diary\n");
    printf("\tnew <note|video>\tAdd a note or a video with current date-time\n");
    printf("\tshow <date|id>\t\tShow entries for specified date or id\n");
    printf("\tlist <date|date-span>\tlist entries for specified date span\n");
    printf("\tplay <id>\t\tPlay specified video\n");
    printf("\tdelete <id>\t\tDelete specified entry\n");
    break;
  }
  exit(1);
}

void init(char *dname) {
  /*
    1. create dir
    2. create reference to diary
    3. create dirtree
    4. create encrypted fs

    .dry/
    storage/  # storage dir
    diaries   # ref file
    dry.conf  # config file
  */

  char *base_folder = "~/.dry";
  char *diary_folder = "storage";
  char fref[1024];
  char dpath[1024];

  sprintf(dpath, "%s/%s/%s", base_folder, diary_folder, dname);
  sprintf(fref, "%s/diaries.ref", base_folder);

  // create dir if not exist
  struct stat st = {0};

  if (stat(dpath, &st) == -1) {
    mkdir(dpath, 0700);
  }
  else { printf("Diary already exist at %s\n", dpath); }

  // add reference to reference file for new diary
  FILE *fd = fopen(fref, "a");
  fprintf(fd, "%s;%s\n", dname, dpath);

  printf("Created new diary %s at %s\n", dname, dpath);
}

typedef enum { ORG, MARKDOWN, TXT } FORMAT;
void newe(char type, char *name) {
  /*
    1. find diary
    2. decrypt diary
    3. create file
    4. encrypt diary
  */

  // Find diary or default if null

  // set default
  if(name == NULL)
    name = "damned";

  // Get current date-time
  time_t timer;
  char buffer[26];
  struct tm *tm_info;

  timer = time(NULL);
  tm_info = localtime(&timer);

  // TODO decrypt diary

  // TODO path to write

  // TODO  mkdir dirtree
  char *storage_path = ".dry/storage";
  // filename format: YYYY-MM-DD.ext

  char file_name[16];
  strftime(file_name, 16, "%Y-%m-%d", tm_info);
  char diary_path[1024];
  char file_path[1024];

  char cmd[1024];
  char *command;

  sprintf(diary_path, "%s/%s", storage_path, name);
  sprintf(file_path, "%s/%s", diary_path, file_name);

  // create file
  printf("Creating new %s\n", type == 'v' ? "video" : "note");
  if(type == 'v') {
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
    command = "ffmpeg -threads 125 -f pulse -ac 2 -i default -thread_queue_size 32 -input_format mjpeg -i /dev/video0 -f mjpeg - %s.mkv 2>/dev/null | ffplay - 2>/dev/null";
  }

  if(type == 'n') {
    FORMAT fmt = ORG; // TODO: possibile formats: markdown, org

    strcat(file_path, ".org");

    FILE *fd;
    struct stat st = {0};
    // Check if file exists and add date time as header
    if (stat(file_path, &st) == -1) {
      strftime(buffer, 26, "%Y-%m-%d", tm_info);
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
    strftime(buffer, 26, "%H:%M:%S", tm_info);

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
    command = "emacsclient -t %s";
  }

  // cmd
  sprintf(cmd, command, file_path);

  //Execute

  system(cmd);
  printf("Written %s\n", "output");
  // encrypt diary
  // TODO
}

void list(char *name) {
  if(name == NULL)
    name = "damned";

  char *c = "exa -hal %s/%s";
  char cmd[1024];
  sprintf(cmd, c, HOMEPATH, name);
  system(cmd);
}

void show(char *id, char *dname) {
  if(dname == NULL)
    dname = "damned";

  char path[1024];
  char cmd[1024];
  char c2[1024];
  char c3[1024];
  char c4[1024];

  sprintf(path, "%s/%s/%s", HOMEPATH, dname, id);
  // Check if file exists
  struct stat st = {0};

  if (stat(path, &st) == -1) {
    printf("Error: file not found\n%s\n", path);
    exit(1);
  }

  // Check file type
  sprintf(c2, "file %s | grep ASCII > /dev/null", path);
  sprintf(c4, "file %s | grep Unicode > /dev/null", path);
  sprintf(c3, "file %s | grep Matroska > /dev/null", path);

  if (system(c2) == 0 || system(c4) == 0)
    sprintf(cmd, "less %s", path);
  else if (system(c3) == 0)
    sprintf(cmd, "mpv %s", path);
  else
    sprintf(cmd, "xdg-open %s", path);

  // Display file
  system(cmd);
}

#define usage(T) usages(argv[0], (T))

int main(int argc, char *argv[], char *envp[]) {
  char *dname = NULL;

  if (argc < 2)
    usage(HELP);

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "init") == 0) {
      if (argc < i + 1) {
	usage(INIT);
      } else
        init(argv[++i]);
    }

    else if (strcmp(argv[i], "new") == 0) {
      if (argc < i + 1) {
	usage(NEW);
      } else {
	// Check type
        char type = 0;

        if (strcmp(argv[i + 1], "video") == 0)
          type = 'v';
        else if (strcmp(argv[i + 1], "note") == 0)
          type = 'n';

	// Get diary or default
	if(argc > 3)
	  dname = argv[3];

	// Call new if type is set
        if (type)
          newe(type, dname);
        else
          perror("Error: wrong type\n");
      }
      break;
    }
    else if (strcmp(argv[i], "list") == 0) {
      // Get diary or default
      if(argc > 4) {
	usage(LIST);
      }

      if(argc > 2)
	dname = argv[2];

      list(dname);
    }
    else if (strcmp(argv[i], "show") == 0) {
      if (argc < i + 2) {
	usage(SHOW);
      } else
	show(argv[i + 1], argv[i + 2]);
    }
  }
  // usage(HELP);
  return 0;
}
