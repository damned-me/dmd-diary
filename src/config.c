/*
 * config.c - Configuration management implementation
 */
#include "config.h"
#include "utils.h"

CONFIG *conf = NULL;

CONFIG *get_config(void) { 
  return conf; 
}

void get_conf_path(char *path) {
  struct passwd *pw = getpwuid(getuid());
  const char *homedir = pw->pw_dir;

  /* Check .dry/dry.conf (current directory) */
  sprintf(path, ".dry/dry.conf");
  if (!do_file_exist(path)) {
    /* Check ~/.dry/dry.conf */
    sprintf(path, "%s/.dry/dry.conf", homedir);
    if (!do_file_exist(path)) {
      /* Check /etc/dry/dry.conf */
      sprintf(path, "/etc/dry/%s", "dry.conf");
      if (!do_file_exist(path)) {
        fprintf(stderr, "Error: can't find config file\n");
        fprintf(stderr,
                "Create config file in\n.dry/dry.conf\n~/.dry/dry.conf\n/etc/dry/dry.conf");
        exit(EXIT_FAILURE);
      }
    }
  }
}

void get_ref_path(char *path) {
  struct passwd *pw = getpwuid(getuid());
  const char *homedir = pw->pw_dir;

  /* Check .dry/diaries.ref (current directory) */
  sprintf(path, ".dry/diaries.ref");
  if (!do_file_exist(path)) {
    /* Check ~/.dry/diaries.ref (home directory) */
    sprintf(path, "%s/.dry/diaries.ref", homedir);
    if (!do_file_exist(path)) {
      /* Create empty reference file (~/.dry/diaries.ref) */
      char dir_path[1024];
      sprintf(dir_path, "%s/.dry", homedir);
      
      struct stat st = {0};
      if (stat(dir_path, &st) == -1) {
        if (mkdir(dir_path, 0700) != 0) {
          fprintf(stderr, "Error: failed to create directory %s\n", dir_path);
          exit(EXIT_FAILURE);
        }
      }
      
      FILE *fd = fopen(path, "w");
      if (fd == NULL) {
        fprintf(stderr, "Error: failed to create reference file %s\n", path);
        exit(EXIT_FAILURE);
      }
      fclose(fd);
      
      fprintf(stderr, "Created reference file at %s\n", path);
    }
  }
}

int config_load(void) {
  config_t cfg;
  config_setting_t *setting;
  const char *str;
  conf = (CONFIG *) calloc(1, sizeof(CONFIG));

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
  
  const char *config_path;
  if(!config_lookup_string(&cfg, "default_dir", &config_path)) {
    fprintf(stderr, "No 'default_dir' setting in configuration file.\n");
  } else {
    /* Expand tilde in path and store it */
    char *expanded_path = (char *)malloc(1024);
    expand_tilde(config_path, expanded_path);
    conf->path = expanded_path;
  }

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
      expand_tilde(value, path);
      fclose(fd);
      return 0;
    }
  }
  fclose(fd);
  return 1;
}
