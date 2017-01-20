#include "main_menu.h"

#include "test_remap.h"

enum {
    APP_MENU_TEST_REMAP,
};

#define MENU_RELOAD 2

static int iteration = 0;
static remap_config_t config;

int ui_app_menu_loop(int cursor_id, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    switch (cursor_id) {
      case APP_MENU_TEST_REMAP:
        ui_test_remap();
        return MENU_RELOAD;
        break;
    }
  }

  return 0;
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

  do {
    iteration++;
    char path[256];
    config_app_path(app, &path);

    if (config_load(path, &config) == -1) {
      config_default(&config);
    }

    struct menu_entry menu[16 + config.size];
    int idx = 0;

    menu[idx++] = (struct menu_entry) { .name = path, .subname = "", .disabled = true, .color = 0xffaa00aa };
    menu[idx++] = (struct menu_entry) { .name = "Test it", .id = APP_MENU_TEST_REMAP };

    menu[idx++] = (struct menu_entry) { .name = "Configure deadzones" };
    char actions_str[256];
    sprintf(actions_str, "Actions: %d", iteration);
    menu[idx++] = (struct menu_entry) { .name = actions_str, .disabled = true, .separator = true };
    menu[idx++] = (struct menu_entry) { .name = "[+] Add trigger" };

    char name[config.size][256];
    for (int i = 0; i < config.size; i++) {
      remap_config_action_name(config, i, &name[i]);
      menu[idx++] = (struct menu_entry) { .name = name[i] };
    }

    struct menu_geom geom = make_geom_centered(500, HEIGHT - 200);
    ret = display_menu(menu, idx, &geom, &ui_app_menu_loop, &ui_app_menu_back, NULL, &app);
  } while (ret != GUI_EXIT);
}
