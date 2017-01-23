#include "main_menu.h"

#include "test_remap.h"

enum {
    APP_MENU_TEST_REMAP = 128,
    APP_MENU_DEADZONES,
};

#define TRIGGER_MENU_OK 2
#define TRIGGER_MENU_CANCEL GUI_EXIT
#define MENU_RELOAD 2

static int iteration = 0;
static remap_config_t config;

int ui_trigger_menu_loop(int cursor_id, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    *(trigger_t *) context = cursor_id;
    return TRIGGER_MENU_OK;
  }

  return GUI_CONTINUE;
}

int ui_trigger_menu(trigger_t *trigger) {
  struct menu_entry menu[TRIGGERS_COUNT + 2];
  int idx;

  menu[idx++] = (struct menu_entry) { .name = "Select button", .disabled = true, .separator = true, .color = 0xffaa00aa };
  for (int i = 0; i < TRIGGERS_COUNT; i++) {
    char *name = remap_trigger_name(TRIGGERS[i]);

    if (name != NULL) {
      menu[idx++] = (struct menu_entry) { .name = name, .id = TRIGGERS[i], };
    }
  }

  struct menu_geom geom = make_geom_centered(400, HEIGHT - 200);
  return display_menu(menu, idx, &geom, &ui_trigger_menu_loop, NULL, NULL, DEFAULT_GUIDE, trigger);
}

int ui_app_menu_loop(int cursor_id, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    switch (cursor_id) {
      case APP_MENU_TEST_REMAP:
        ui_test_remap(config);
        return MENU_RELOAD;
    }
  }

  if (was_button_pressed(SCE_CTRL_TRIANGLE)) {
    trigger_t trigger;

    if (ui_trigger_menu(&trigger) == TRIGGER_MENU_OK) {
      config_append_remap(&config);
      config.triggers[config.size - 1] = trigger;
    }

    return MENU_RELOAD;
  }

  if (was_button_pressed(SCE_CTRL_SQUARE) && cursor_id < config.size) {
    config_remove_remap(&config, cursor_id);

    sceKernelDelayThread(500 * 1000);
    return MENU_RELOAD;
  }

  return GUI_CONTINUE;
}

int ui_app_menu_back(void *context) {
  application_t *app = (application_t *) context;
  char path[256];
  config_app_path(*app, &path);

  config_save(path, config);
  return GUI_EXIT;
}

int ui_app_menu(application_t app) {
  int ret = 0;
  iteration++;
  char path[256];
  config_app_path(app, &path);

  if (config_load(path, &config) == -1) {
    config_default(&config);
  }

  do {
    struct menu_entry menu[16 + config.size];
    int idx = 0;

    menu[idx++] = (struct menu_entry) { .name = path, .subname = "", .disabled = true, .separator = true, .color = 0xffaa00aa };
    menu[idx++] = (struct menu_entry) { .name = "Test remap", .id = APP_MENU_TEST_REMAP };

    menu[idx++] = (struct menu_entry) { .name = "Configure deadzones", .id = APP_MENU_DEADZONES };
    char actions_str[256];
    sprintf(actions_str, "Actions: %d", config.size);
    menu[idx++] = (struct menu_entry) { .name = actions_str, .disabled = true, .separator = true };
    //menu[idx++] = (struct menu_entry) { .name = "[+] Add", .id = APP_MENU_ADD_REMAP };

    char name[config.size][256];
    for (int i = 0; i < config.size; i++) {
      remap_config_action_name(config, i, &name[i]);
      menu[idx++] = (struct menu_entry) { .name = name[i], .id = i };
    }

    struct menu_geom geom = make_geom_centered(500, HEIGHT - 200);
    ret = display_menu(menu, idx, &geom, &ui_app_menu_loop, &ui_app_menu_back, NULL, EXT_GUIDE(ICON_SQUARE("delete"), ICON_TRIANGLE("add")), &app);
  } while (ret != GUI_EXIT);
}
