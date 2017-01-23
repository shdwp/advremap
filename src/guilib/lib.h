#ifndef GUILIB_H
#define GUILIB_H

#include <stdbool.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <vita2d.h>

#define WIDTH 960
#define HEIGHT 544

#define GUI_CONTINUE 0
#define GUI_EXIT 1

#define ICON_CROSS(a) "╳" a
#define ICON_CIRCLE(a) "◯" a
#define ICON_TRIANGLE(a) "△" a
#define ICON_SQUARE(a) "□" a

#define DEFAULT_GUIDE (char *[]) {ICON_CROSS("ok"), ICON_CIRCLE("back"), NULL, NULL}
#define EXT_GUIDE(s, t) (char *[]) {ICON_CROSS("ok"), ICON_CIRCLE("back"), s, t}

struct menu_entry {
  int id;
  unsigned int color;
  char *name, *suffix;
  char *subname;
  bool disabled, separator;
};

struct menu_geom {
  int x, y, width, height, el, total_y;
  bool statusbar;
};

vita2d_pgf *gui_font;

#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)

struct menu_geom make_geom_centered(int w, int h);

typedef int (*gui_loop_callback) (int, void *);
typedef int (*gui_back_callback) (void *);
typedef void (*gui_draw_callback) (void);

bool was_button_pressed(short id);
bool is_button_down(short id);
bool is_rectangle_touched(int lx, int ly, int rx, int ry);

int display_menu(
        struct menu_entry menu[],
        int total_elements,
        struct menu_geom *geom_ptr,
        gui_loop_callback loop_callback,
        gui_back_callback back_callback,
        gui_draw_callback draw_callback,
        char *guide_strings[],
        void *context
        );

void display_alert(
        char *message,
        char *button_captions[],
        int buttons_count,
        gui_loop_callback cb,
        void *context
        );

void display_error(char *format, ...);

void flash_message(char *format, ...);

void guilib_init(gui_loop_callback global_loop_cb, gui_draw_callback global_draw_cb);

#endif
