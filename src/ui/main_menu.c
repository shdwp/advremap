#include "main_menu.h"

#include "test_remap.h"

enum {
    MAIN_MENU_TEST_REMAP,
};

int ui_main_menu_loop(int cursor_id, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    switch (cursor_id) {
      case MAIN_MENU_TEST_REMAP:
        ui_test_remap();
        break;
    }
  }

  return 0;
}

int ui_main_menu_back(void *context) {
  return GUI_CONTINUE;
}

int ui_main_menu() {
  struct menu_entry menu[16];
  int idx = 0;

  menu[idx++] = (struct menu_entry) { .name = "", .subname = "Advanced Button Remap", .disabled = true, .color = 0xffaa00aa };
  char name[256];

  menu[idx++] = (struct menu_entry) { .name = "Test remap", .id = MAIN_MENU_TEST_REMAP };
  menu[idx++] = (struct menu_entry) { .name = "Enable applications" };
  menu[idx++] = (struct menu_entry) { .name = "Configure buttons" };
  menu[idx++] = (struct menu_entry) { .name = "Quit" };

  struct menu_geom geom = make_geom_centered(500, 200);
  return display_menu(menu, idx, &geom, &ui_main_menu_loop, &ui_main_menu_back, NULL, NULL);
}
