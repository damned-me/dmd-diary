#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wait.h>
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

  char *base_folder = ".dry";
  char *diary_folder = "storage";
  char fref[1024];
  char dpath[1024];

  sprintf(dpath, "%s/storage/%s", base_folder, dname);
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

  // TODO decrypt diary

  // TODO path to write

  // TODO dirtree
  char *path = ".dry/storage";
  char ep[1024];
  sprintf(ep, "%s/%s", path, name);

  // create file
  printf("Creating new %s\n", type == 'v' ? "video" : "note");
  char *command;

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
    command = "ffmpeg -threads 125 -f pulse -ac 2 -i default -thread_queue_size 32 -input_format mjpeg -i /dev/video0 -f mjpeg - %s/%s.mkv 2>/dev/null | ffplay - 2>/dev/null";
  }

  if(type == 'n') {
    /* call file and editor */
    command = "emacsclient %s/%s";
  }

  char ex[1024];

  // create name for file
  sprintf(ex, command, ep, "output");

  //Execute
  int pid = fork();
  if(pid < 0) {
    fprintf(stderr, "Error: fork returned < 0");
    exit(1);
  }

  if(pid == 0) {
    // Child
    system(ex);
    printf("Written %s\n", "output");
    exit(0);
  }
  wait(NULL);

  // encrypt diary
  // TODO
}

void list(char *name) {
  if(name == NULL)
    name = "damned";

  char *c = "exa -hal %s/%s";
  char cmd[1024];
  sprintf(cmd, c, ".dry/storage", name);
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

  sprintf(path, "%s/%s/%s", ".dry/storage", dname, id);

  // Check if file exists
  struct stat st = {0};

  if (stat(path, &st) == -1) {
    printf("Error: file not found\n");
    exit(1);
  }

  // Check file type
  sprintf(c2, "file %s | grep ASCII > /dev/null", path);
  sprintf(c4, "file %s | grep Unicode > /dev/null", path);
  sprintf(c3, "file %s | grep Matroska > /dev/null", path);

  if(system(c2) == 0 || system(c4) == 0)
    sprintf(cmd, "less %s", path);
  if(system(c3) == 0)
    sprintf(cmd, "mpv %s", path);

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
  return 0;
}
