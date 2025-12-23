/*
 * crypto.c - Encryption/decryption operations implementation
 */
#include "crypto.h"
#include "config.h"
#include "utils.h"

void encdiary(int opcl, const char *name, const char *base_path) {
  /*
   * Encryption (encfs)
   * 
   * Open:  encfs <encrypted_dir> <mount_point>
   * Close: fusermount -u <mount_point>
   * 
   * The encrypted directory is stored as .<name> in the parent of mount_point
   * e.g., mount_point = /path/to/storage/diary
   *       enc_path    = /path/to/storage/.diary
   *
   * Environment variables:
   *   DRY_ENCFS_PASSWORD - If set, use --extpass to provide password non-interactively
   *   DRY_NO_UNMOUNT     - If set to "1", skip unmounting (useful for testing)
   */
  char cmd[8192];
  char enc_path[2048];
  char mount_point[2048];
  
  if (name == NULL)
    name = get_config()->name;

  if (base_path == NULL)
    base_path = get_config()->path;

  /* Construct mount_point and enc_path */
  snprintf(mount_point, sizeof(mount_point), "%s/%s", base_path, name);
  snprintf(enc_path, sizeof(enc_path), "%s/.%s", base_path, name);

  if (!opcl) {
    /* OPEN: decrypt and mount */
    
    /* check if encrypted source exists */
    if (!do_file_exist(enc_path)) {
      fprintf(stderr, "Error: encrypted directory %s does not exist\n", enc_path);
      fprintf(stderr, "Please initialize the diary first with: dry init %s\n", name);
      exit(EXIT_FAILURE);
    }
    
    /* check if already mounted */
    snprintf(cmd, sizeof(cmd), "mountpoint -q %s 2>/dev/null", mount_point);
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
    const char *extpass = getenv("DRY_ENCFS_PASSWORD");
    if (extpass != NULL && extpass[0] != '\0') {
      /* Use --extpass for non-interactive mode (testing/scripting) */
      snprintf(cmd, sizeof(cmd), "encfs --extpass='echo %s' %s %s", extpass, enc_path, mount_point);
    } else {
      snprintf(cmd, sizeof(cmd), "encfs %s %s", enc_path, mount_point);
    }
    if (system(cmd) != 0) {
      fprintf(stderr, "Error: failed to mount encrypted filesystem\n");
      rmdir(mount_point);
      exit(EXIT_FAILURE);
    }
  }
  else {
    /* CLOSE: unmount and cleanup */
    
    /* Check if unmounting is disabled (for testing) */
    const char *no_unmount = getenv("DRY_NO_UNMOUNT");
    if (no_unmount != NULL && strncmp(no_unmount, "1", 2) == 0) {
      return;
    }
    
    /* check if mounted */
    snprintf(cmd, sizeof(cmd), "mountpoint -q %s 2>/dev/null", mount_point);
    if (system(cmd) != 0) {
      /* not mounted, just cleanup */
      rmdir(mount_point);
      return;
    }
    
    /* unmount */
    snprintf(cmd, sizeof(cmd), "fusermount -u %s", mount_point);
    if (system(cmd) != 0) {
      fprintf(stderr, "Warning: failed to unmount %s\n", mount_point);
    }
    
    /* remove mount point directory */
    rmdir(mount_point);
  }
}
