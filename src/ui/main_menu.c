#include "main_menu.h"

#include "test_remap.h"

int ui_main_menu_loop(int cursor_id, void *context) {
  if (was_button_pressed(SCE_CTRL_CROSS)) {
    application_t *apps = (application_t *) context;
    ui_app_menu(apps[cursor_id]);
  }

  return 0;
}

int ui_main_menu_back(void *context) {
  return GUI_CONTINUE;
}

int ui_main_menu() {
  int app_count = 4;
  application_t apps[4] = {
    {.name = "LEGEND OF HEROES: TRAILS OF COLD STEEL", .id = "PCSB00866", },
    {.name = "Persona 4: Dancing All Night", .id = "PCSB00867", },
    {.name = "???", .id = "PCSB00743", },
    {.name = "Ys: Memories of Celceta", .id = "PCSB00497", },
  };

  struct menu_entry menu[3 + app_count];
  int idx = 0;

  menu[idx++] = (struct menu_entry) { .name = "", .subname = "Advanced Button Remap", .disabled = true, .color = 0xffaa00aa };
  for (int i = 0; i < app_count; i++) {
    menu[idx++] = (struct menu_entry) { .name = apps[i].name, .id = i, };
  }

  menu[idx++] = (struct menu_entry) { .name = "", .disabled = true, .separator = true };
  menu[idx++] = (struct menu_entry) { .name = "Quit" };

  struct menu_geom geom = make_geom_centered(500, 200);
  return display_menu(menu, idx, &geom, &ui_main_menu_loop, &ui_main_menu_back, NULL, DEFAULT_GUIDE, &apps);
}
