#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include "remap.h"

typedef struct application {
    char *name;
    char *id;
} application_t;

void config_app_path(application_t app, char *path);
int config_save(char *path, remap_config_t config);
int config_load(char *path, remap_config_t *result);
int config_default(remap_config_t *config);

#endif
