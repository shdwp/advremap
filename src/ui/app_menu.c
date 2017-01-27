#include "app_menu.h"
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
static SceRtcTick wait_until_tick;
static deadzone_loop_setup = false;

// deadzone

int ui_deadzone_menu_loop(int cursor_id, void *context) {
  bool did_change = false;

  bool left = was_button_pressed(SCE_CTRL_LEFT), right = was_button_pressed(SCE_CTRL_RIGHT);
  if (left || right) {
    int delta = left ? -15 : (right ? 15 : 0);

    switch (cursor_id) {
      case 1: config.back_touch_deadzone_vertical += delta; break;
      case 2: config.back_touch_deadzone_horizontal += delta; break;
      case 3: config.front_touch_deadzone_vertical += delta; break;
      case 4: config.front_touch_deadzone_horizontal += delta; break;
      case 5: config.rs_deadzone += delta; break;
      case 6: config.ls_deadzone += delta; break;
    }

    did_change = true;
  }

  if (did_change || deadzone_loop_setup) {
    int numbers[] = { config.back_touch_deadzone_vertical, config.back_touch_deadzone_horizontal, config.front_touch_deadzone_vertical, config.front_touch_deadzone_horizontal, config.rs_deadzone, config.ls_deadzone };
    struct menu_entry *menu = context;

    for (int i = 0; i < sizeof(numbers) / sizeof(int); i++) {
      sprintf(menu[i + 1].subname, "%dpx", numbers[i]);
    }

    deadzone_loop_setup = false;
  }

  return GUI_CONTINUE;
}

void ui_deadzone_menu_draw(void *context) {
  SceCtrlData pad;
  SceTouchData front;
  SceTouchData back;

  sceCtrlPeekBufferPositive(0, &pad, 1);
  sceTouchPeek(SCE_TOUCH_PORT_FRONT, &front, 1);
  sceTouchPeek(SCE_TOUCH_PORT_BACK, &back, 1);

  remap_deadzone_ignore(config, &pad, &front, &back);

  draw_touch_at(front, 300, 250);
  draw_touch_at(back, 300, 370);
  draw_sticks_at(pad, 0, 370);
}

int ui_deadzone_menu() {
  deadzone_loop_setup = true;
  struct menu_entry menu[16];
  int idx = 0;

  char subnames[6][128];

  menu[idx++] = (struct menu_entry) { .name = "Deadzones", .disabled = true, .separator = true, .color = 0xffaa00aa };
  menu[idx++] = (struct menu_entry) { .name = "Back touch vertical", .subname = subnames[0], .suffix = "←→", .id = 1  };
  menu[idx++] = (struct menu_entry) { .name = "Back touch horizontal", .subname = subnames[1], .suffix = "←→", .id = 2 };
  menu[idx++] = (struct menu_entry) { .name = "Front touch vertical", .subname = subnames[2], .suffix = "←→", .id = 3 };
  menu[idx++] = (struct menu_entry) { .name = "Front touch horizontal", .subname = subnames[3], .suffix = "←→", .id = 4 };

  menu[idx++] = (struct menu_entry) { .name = "RS deadzone", .subname = subnames[4], .suffix = "←→", .id = 5 };
  menu[idx++] = (struct menu_entry) { .name = "LS deadzone", .subname = subnames[5], .suffix = "←→", .id = 6 };

  struct menu_geom geom = make_geom_centered(400, 180);
  geom.y = 30;
  return display_menu(
      menu,
      idx,
      &geom,
      &ui_deadzone_menu_loop,
      NULL,
      &ui_deadzone_menu_draw,
      DEFAULT_GUIDE,
      (void *) menu
      );
}

// action
int ui_action_menu_loop(int cursor_id, void *context) {
  SceRtcTick current_tick;
  sceRtcGetCurrentTick(&current_tick);
  if (current_tick.tick > wait_until_tick.tick) {
    SceCtrlData pad;
    SceTouchData front;
    SceTouchData back;

    sceCtrlPeekBufferPositive(0, &pad, 1);
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &front, 1);
    sceTouchPeek(SCE_TOUCH_PORT_BACK, &back, 1);

    remap_deadzone_ignore(config, &pad, &front, &back);

    if (remap_read_actions((action_list_t *) context, pad, front, back) == 0) {
      return TRIGGER_MENU_OK;
    } else {
      return TRIGGER_MENU_CANCEL;
    }
  }

  return GUI_CONTINUE;
}

int ui_action_menu(action_list_t *actions) {
  sceRtcGetCurrentTick(&wait_until_tick);
  wait_until_tick.tick += 3000 * 1000;

  char *alert_text = "Hold the buttons to replace with\nuntil this message dissapear.\nDont touch anything to cancel.";
  return display_alert(
      alert_text,
      make_geom_centered(400, 200),
      NULL,
      0,
      &ui_action_menu_loop,
      (void *) actions
      );
}

// trigger
int ui_trigger_menu_loop(int cursor_id, void *context) {
  SceRtcTick current_tick;
  sceRtcGetCurrentTick(&current_tick);
  if (current_tick.tick > wait_until_tick.tick) {
    SceCtrlData pad;
    SceTouchData front;
    SceTouchData back;

    sceCtrlPeekBufferPositive(0, &pad, 1);
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &front, 1);
    sceTouchPeek(SCE_TOUCH_PORT_BACK, &back, 1);

    remap_deadzone_ignore(config, &pad, &front, &back);
    if (remap_read_trigger(context, pad, front, back) == 0) {
      return TRIGGER_MENU_OK;
    }
  }

  return GUI_CONTINUE;
}

int ui_trigger_menu(trigger_t *trigger) {
  sceRtcGetCurrentTick(&wait_until_tick);
  wait_until_tick.tick += 300 * 1000;

  char *alert_text = "Press the button which\nwill be replaced.";
  return display_alert(
      alert_text,
      make_geom_centered(400, 200),
      NULL,
      0,
      &ui_trigger_menu_loop,
      (void *) trigger
      );
}

// app menu
int ui_app_menu_loop(int cursor_id, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    switch (cursor_id) {
      case APP_MENU_TEST_REMAP:
        ui_test_remap(config);
        break;
      case APP_MENU_DEADZONES:
        ui_deadzone_menu();
        break;
    }
  }

  if (was_button_pressed(SCE_CTRL_TRIANGLE)) {
    trigger_t trigger;

    action_t *list = malloc(sizeof(action_t) * REMAP_MAX_ACTIONS);
    action_list_t actions = {
      .size = 0,
      .list = list,
    };

    if (ui_trigger_menu(&trigger) != TRIGGER_MENU_OK) {
      return MENU_RELOAD;
    }

    if (ui_action_menu(&actions) != TRIGGER_MENU_OK) {
      return MENU_RELOAD;
    }

    config_append_remap(&config);
    config.triggers[config.size - 1] = trigger;
    config.actions[config.size - 1] = actions;

    flash_message("Action added!");
    sceKernelDelayThread(2500 * 1000);

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
  if (config.size > 0) {
    application_t *app = (application_t *) context;
    char path[256], binary_path[CONFIG_APP_PATH_SIZE];
    config_path(*app, &path);
    config_binary_path(*app, &binary_path);

    config_save(path, config);
    config_binary_install(binary_path);
    config_binary_save(binary_path, path);

    int taihen_append_result = config_taihen_append(*app);
    char *alert_text;
    if (taihen_append_result  == 1) {
      alert_text = "Plugin was not enabled previously, so:\n\
1. start moleculeShell\n\
2. open ux0:tai/config.txt\n\
3. uncomment two last lines (remove #)\n\
4. save the file\n\
5. press Start and select\n \"Reload taiHEN config.txt\"\n\
\n\
After this you may start the game!";
    } else if (taihen_append_result == 2) {
      alert_text = "Configuration was updated, but not enabled:\n\
1. start moleculeShell\n\
2. open ux0:tai/config.txt\n\
3. uncomment two last lines (remove #)\n\
4. save the file\n\
5. press Start and select\n \"Reload taiHEN config.txt\"\n\
\n\
After this you may start the game!";
    }

    if (taihen_append_result > 0) {
      display_alert(
          alert_text,
          make_geom_centered(500, HEIGHT - 230),
          NULL,
          1,
          NULL,
          NULL
          );
    }
  }

  return GUI_EXIT;
}

int ui_app_menu(application_t app) {
  int ret = 0;
  iteration++;
  char path[256];
  config_path(app, &path);
  taiReloadConfig();

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

    char name[config.size][1024];
    for (int i = 0; i < config.size; i++) {
      remap_config_action_name(config, i, &name[i]);
      menu[idx++] = (struct menu_entry) { .name = name[i], .id = i };
    }

    struct menu_geom geom = make_geom_centered(500, HEIGHT - 200);
    ret = display_menu(menu, idx, &geom, &ui_app_menu_loop, &ui_app_menu_back, NULL, EXT_GUIDE(ICON_SQUARE("delete"), ICON_TRIANGLE("add")), &app);
  } while (ret != GUI_EXIT);
}
