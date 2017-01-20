#include <psp2/ctrl.h>
#include <psp2/rtc.h>
#include <psp2/touch.h>

#include <vita2d.h>

#include "ui/main_menu.h"

static vita2d_pgf *pgf;

void main() {
    vita2d_init();
    vita2d_set_clear_color(RGBA8(0, 0, 0, 0));
    guilib_init(NULL, NULL);
    ui_main_menu();
}
