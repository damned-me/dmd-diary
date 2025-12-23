/*
 * config.h - Configuration management
 */
#ifndef CONFIG_H
#define CONFIG_H

#include "dry.h"
#include <libconfig.h>

/* Global config pointer */
extern CONFIG *conf;

/* Get the global config */
CONFIG *get_config(void);

/* Load configuration from file */
int config_load(void);

/* Get path to config file */
void get_conf_path(char *path);

/* Get path to reference file */
void get_ref_path(char *path);

/* Get diary path by name from reference file */
int get_path_by_name(const char *dname, char *path);

#endif /* CONFIG_H */
