#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/*
Webcam recordings (ffmpeg)
https://askubuntu.com/questions/1445157/how-to-record-webcam-with-audio-on-ubuntu-22-04-from-cli
https://ffmpeg.org/

ffmpeg -f pulse -ac 2 -i default -f v4l2 -i /dev/video0 -t 00:00:20 -vcodec
libx264 record.mp4 The -f defines the format, pulse for audio and v4l2(Video 4
Linux 2). The -ac defines the audio channels, in this case 2. The -i defines the
input, default for audio and the webcam for video. The -t defines the duration
of the recording, in this case 20 seconds. The -vcodec defines the output video
codec, since it is an mp4 file it is set to libx264 (H.264) Audio should default
to AAC so -acodec is not needed.

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

void usage(char *name) {
  printf("Usage: %s [-v | --version] [-h | --help] <command> [<args>]\n", name);
  printf("\nCOMMANDS\n");
  printf("\tinit\t\tInitialize a new diary\n");
  printf("\tnew <note|video>\t\tAdd a note or a video with current date-time\n");
  printf("\tshow <date|id>\t\tShow entries for specified date or id\n");
  printf("\tlist <date|date-span>\t\tlist entries for specified date span\n");
  printf("\tplay <id>\t\tPlay specified video\n");
  printf("\tdelete <id>\t\tDelete specified entry\n");
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
  char *fref = "diaries";
  char dpath[1024];

  sprintf(dpath, "%s/%s", base_folder, dname);
  sprintf(fref, "%s/%s", base_folder, fref);

  // create dir if not exist
  struct stat st = {0};

  if (stat(dpath, &st) == -1) {
    mkdir(dpath, 0700);
  }

  // add reference to reference file for new diary
  FILE *fd = fopen(fref, "a");
  fprintf(fd, "%s\n", dpath);

  // create dirtree
  <
}

void new(char *type) {
  /*
    1. find diary
    2. decrypt diary
    3. create file
    4. encrypt diary
   */
}

int main(int argc, char *argv[], char *envp[]) {
  if (argc < 2)
    usage(argv[0]);

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "init") == 0) {
      init(argv[++i]);
    }
    if (strcmp(argv[i], "new") == 0) {
      new(argv[++i]);
    }
  }
  return 0;
}
