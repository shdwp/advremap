#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include "remap.h"
#include "../applist/applist.h"

#define CONFIG_VERSION 4
#define CONFIG_APP_PATH_SIZE 256
#define CONFIG_PLUGIN_PATH "app0:advremap.suprx"
#define CONFIG_TAIHEN_PATH "ux0:tai/config.txt"

#define CONFIG_MEM_MAX_SIZE 4096
#define CONFIG_MEM_OFFSET 70332

void config_path(application_t app, char path[CONFIG_APP_PATH_SIZE]);
void config_binary_path(application_t app, char path[CONFIG_APP_PATH_SIZE]);

int config_load(char *path, remap_config_t *result);
int config_mem_load(void *ptr, remap_config_t *result);
int config_save(char *path, remap_config_t config);

int config_binary_save(char *binary_path, char *config_path);
int config_binary_install(char *name);

int config_default(remap_config_t *config);

int config_taihen_append(application_t app);

void config_append_remap(remap_config_t *config);
void config_remove_remap(remap_config_t *config, int n);

#endif
