#include "config.h"

#include <psp2/io/fcntl.h>

#define FREAD_INTO(into, stream) fread(&into, sizeof(into), 1, stream)
#define FWRITE_FROM(from, stream) fwrite(&from, sizeof(from), 1, stream)

#define VOID_FOPEN() size_t v_offset = 0;
#define VOID_FREAD_INTO(type, into, ptr) into = *((type *) (ptr + v_offset)); v_offset += sizeof(type);
#define VOID_FREAD(type, into, size, ptr) into = ((type *) (ptr + v_offset)); v_offset += size;

#define CONFIG_VERSION 1

void config_path(application_t app, char path[CONFIG_APP_PATH_SIZE]) {
    sprintf(path, "ux0:data/advremap/%s", app.id);
}

void config_binary_path(application_t app, char path[CONFIG_APP_PATH_SIZE]) {
    sprintf(path, "ux0:tai/advremap/%s.suprx", app.id);
}

int config_load(char *path, remap_config_t *result) {
    FILE *file = fopen(path, "r");
    if (!file) {
        return -1;
    }

    int version = 0;
    FREAD_INTO(version, file);
    if (version != CONFIG_VERSION) {
        return -1;
    }

    // read deadzones
    FREAD_INTO(result->rs_deadzone, file);
    FREAD_INTO(result->ls_deadzone, file);
    FREAD_INTO(result->back_touch_deadzone_vertical, file);
    FREAD_INTO(result->back_touch_deadzone_horizontal, file);
    FREAD_INTO(result->front_touch_deadzone_vertical, file);
    FREAD_INTO(result->front_touch_deadzone_horizontal, file);

    // read triggers
    FREAD_INTO(result->size, file);
    result->triggers = malloc(sizeof(trigger_t) * result->size);
    fread(result->triggers, sizeof(trigger_t), result->size, file);

    // read actions
    result->actions = malloc(sizeof(action_list_t) * result->size);
    for (int i = 0; i < result->size; i++) {
        FREAD_INTO(result->actions[i].size, file);
        result->actions[i].list = malloc(sizeof(action_t) * result->actions[i].size);
        fread(result->actions[i].list, sizeof(action_t), result->actions[i].size, file);
    }

    fclose(file);
    return 0;
}

int config_save(char *path, remap_config_t config) {
    FILE *file = fopen(path, "w");

    int version = CONFIG_VERSION;
    FWRITE_FROM(version, file);
    FWRITE_FROM(config.rs_deadzone, file);
    FWRITE_FROM(config.ls_deadzone, file);
    FWRITE_FROM(config.back_touch_deadzone_vertical, file);
    FWRITE_FROM(config.back_touch_deadzone_horizontal, file);
    FWRITE_FROM(config.front_touch_deadzone_vertical, file);
    FWRITE_FROM(config.front_touch_deadzone_horizontal, file);

    FWRITE_FROM(config.size, file);
    fwrite(config.triggers, sizeof(trigger_t), config.size, file);
    for (int i = 0; i < config.size; i++) {
        FWRITE_FROM(config.actions[i].size, file);
        fwrite(config.actions[i].list, sizeof(action_t), config.actions[i].size, file);
    }

    fclose(file);

    return 0;
}

int config_mem_save(char *binary_path, char *config_path) {
    void *config_mem = malloc(CONFIG_MEM_MAX_SIZE);
    FILE *config_file = fopen(config_path, "r");
    fread(config_mem, CONFIG_MEM_MAX_SIZE, 1, config_file);
    fclose(config_file);

    // it looks like fseek doesn't work, so we use sceIo here
    FILE *binary_file = sceIoOpen(binary_path, SCE_O_WRONLY, 0777);
    sceIoLseek(binary_file, CONFIG_MEM_OFFSET, SCE_SEEK_SET);
    sceIoWrite(binary_file, config_mem, CONFIG_MEM_MAX_SIZE);

    sceIoClose(binary_file);

    return 0;
}

int config_mem_load(void *ptr, remap_config_t *result) {
    VOID_FOPEN();

    int version = 0;
    VOID_FREAD_INTO(int, version, ptr);
    if (version != CONFIG_VERSION) {
        return -1;
    }

    // read deadzones
    VOID_FREAD_INTO(int, result->rs_deadzone, ptr);
    VOID_FREAD_INTO(int, result->ls_deadzone, ptr);
    VOID_FREAD_INTO(int, result->back_touch_deadzone_vertical, ptr);
    VOID_FREAD_INTO(int, result->back_touch_deadzone_horizontal, ptr);
    VOID_FREAD_INTO(int, result->front_touch_deadzone_vertical, ptr);
    VOID_FREAD_INTO(int, result->front_touch_deadzone_horizontal, ptr);

    VOID_FREAD_INTO(int, result->size, ptr);
    result->triggers = malloc(sizeof(trigger_t) * result->size);
    VOID_FREAD(trigger_t, result->triggers, sizeof(trigger_t) * result->size, ptr);

    // read actions
    result->actions = malloc(sizeof(action_list_t) * result->size);
    for (int i = 0; i < result->size; i++) {
        VOID_FREAD_INTO(int, result->actions[i].size, ptr);
        result->actions[i].list = malloc(sizeof(action_t) * result->actions[i].size);

        VOID_FREAD(action_t, result->actions[i].list, sizeof(action_t) * result->actions[i].size, ptr);
    }

    return 0;
}

int config_default(remap_config_t *config) {
    config->rs_deadzone = 0;
    config->ls_deadzone = 0;
    config->back_touch_deadzone_vertical = 0;
    config->back_touch_deadzone_horizontal = 0;
    config->front_touch_deadzone_vertical = 0;
    config->front_touch_deadzone_horizontal = 0;
    config->size = 0;

    return 0;
}

void config_append_remap(remap_config_t *config) {
    int i = config->size++;
    config->triggers = realloc(config->triggers, sizeof(trigger_t) * config->size);
    config->triggers[i] = CTRL_CROSS;

    action_list_t list = {
        .size = 0,
        .list = 0,
    };

    config->actions = realloc(config->actions, sizeof(action_list_t) * config->size);
    config->actions[i] = list;
}

void config_remove_remap(remap_config_t *config, int n) {
    for (; n < config->size - 1; n++) {
        config->actions[n] = config->actions[n+1];
        config->triggers[n] = config->triggers[n+1];
    }

    config->size--;
    config->triggers = realloc(config->triggers, sizeof(trigger_t) * config->size);
    config->actions = realloc(config->actions, sizeof(action_list_t) * config->size);
}
