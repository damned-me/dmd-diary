/*
 * crypto.c - Encryption/decryption operations implementation
 */
#include "crypto.h"
#include "config.h"
#include "utils.h"

void encdiary(int opcl, const char *name, const char *path) {
  /*
   * Encryption (encfs)
   * 
   * Open:  encfs <encrypted_dir> <mount_point>
   * Close: fusermount -u <mount_point>
   */
  char cmd[2048];
  char enc_path[1024];
  char mount_point[1024];
  
  if (name == NULL)
    name = get_config()->name;

  if (path == NULL)
    path = get_config()->path;

  sprintf(enc_path, "%s/.%s", path, name);
  sprintf(mount_point, "%s/%s", path, name);

  if (!opcl) {
    /* OPEN: decrypt and mount */
    
    /* check if encrypted source exists */
    if (!do_file_exist(enc_path)) {
      fprintf(stderr, "Error: encrypted directory %s does not exist\n", enc_path);
      fprintf(stderr, "Please initialize the diary first with: dry init %s\n", name);
      exit(EXIT_FAILURE);
    }
    
    /* check if already mounted */
    sprintf(cmd, "mountpoint -q %s 2>/dev/null", mount_point);
    if (system(cmd) == 0) {
      /* already mounted, nothing to do */
      return;
    }
    
    /* prepare clean mount point */
    rmdir(mount_point);  /* remove if empty */
    if (mkdir(mount_point, 0700) != 0 && errno != EEXIST) {
      fprintf(stderr, "Error: failed to create mount point %s: %s\n", mount_point, strerror(errno));
      exit(EXIT_FAILURE);
    }
    
    /* mount encrypted filesystem */
    sprintf(cmd, "encfs %s %s", enc_path, mount_point);
    if (system(cmd) != 0) {
      fprintf(stderr, "Error: failed to mount encrypted filesystem\n");
      rmdir(mount_point);
      exit(EXIT_FAILURE);
    }
  }
  else {
    /* CLOSE: unmount and cleanup */
    
    /* check if mounted */
    sprintf(cmd, "mountpoint -q %s 2>/dev/null", mount_point);
    if (system(cmd) != 0) {
      /* not mounted, just cleanup */
      rmdir(mount_point);
      return;
    }
    
    /* unmount */
    sprintf(cmd, "fusermount -u %s", mount_point);
    if (system(cmd) != 0) {
      fprintf(stderr, "Warning: failed to unmount %s\n", mount_point);
    }
    
    /* remove mount point directory */
    rmdir(mount_point);
  }
}
