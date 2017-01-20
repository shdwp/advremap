#include "config.h"

#define FREAD_INTO(into, stream) fread(&into, sizeof(into), 1, stream)

/*
 * File structure:
 * int rs_deadzone, ls_deadzone;
   int back_touch_deadzone_vertical, back_touch_deadzone_horizontal;
   int front_touch_deadzone_vertical, front_touch_deadzone_horizontal;
 *
 *
 */
remap_config_t config_load(char *path) {
    remap_config_t result = {};
    FILE *file = fopen(path, "r");

    // read deadzones
    FREAD_INTO(result.rs_deadzone, file);
    FREAD_INTO(result.rs_deadzone, file);
    FREAD_INTO(result.ls_deadzone, file);
    FREAD_INTO(result.back_touch_deadzone_vertical, file);
    FREAD_INTO(result.back_touch_deadzone_horizontal, file);
    FREAD_INTO(result.front_touch_deadzone_vertical, file);
    FREAD_INTO(result.front_touch_deadzone_horizontal, file);

    // read triggers
    result.triggers = malloc(sizeof(trigger_t) * result.size);
    fread(result.triggers, sizeof(trigger_t), result.size, file);

    // read actions
    result.actions = malloc(sizeof(action_list_t) * result.size);
    for (int i = 0; i < result.size; i++) {
        FREAD_INTO(result.actions[i].size, file);

        for (int n = 0; n < result.actions[i].size; n++) {
            result.actions[i].list = malloc(sizeof(action_t) * result.actions[i].size);
            fread(result.actions[i].list, sizeof(action_t), result.actions[i].size, file);
        }
    }

    fclose(file);
}

config_save(char *path, remap_config_t config) {

}
