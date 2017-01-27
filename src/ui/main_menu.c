#include "main_menu.h"

#include "test_remap.h"

int ui_main_menu_loop(int cursor_id, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    applist_t *list = (applist_t *) context;
    ui_app_menu(list->items[cursor_id]);
  }

  return 0;
}

int ui_main_menu_back(void *context) {
  return GUI_CONTINUE;
}

int ui_main_menu() {
  applist_t list;
  applist_load(&list);

  struct menu_entry menu[3 + list.size];
  int idx = 0;

  menu[idx++] = (struct menu_entry) { .name = "", .subname = "Advanced Button Remap", .disabled = true, .color = 0xffaa00aa };
  for (int i = 0; i < list.size; i++) {
    menu[idx++] = (struct menu_entry) { .name = list.items[i].name, .id = i, };
  }

  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .separator = true };
  menu[idx++] = (struct menu_entry) { .name = "Quit" };

  struct menu_geom geom = make_geom_centered(600, 400);
  return display_menu(menu, idx, &geom, &ui_main_menu_loop, &ui_main_menu_back, NULL, DEFAULT_GUIDE, &list);
}
