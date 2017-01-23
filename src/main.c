#include <psp2/ctrl.h>
#include <psp2/rtc.h>
#include <psp2/touch.h>

#include <vita2d.h>

#include "ui/main_menu.h"
#include "debugnet.h"

static vita2d_pgf *pgf;

#define ip_server "192.168.0.103"
#define port_server 18194
void main() {
    debugNetInit(ip_server,port_server,DEBUG);
    debugNetPrintf(DEBUG, "Start\n");

    vita2d_init();
    vita2d_set_clear_color(RGBA8(0, 0, 0, 0));
    guilib_init(NULL, NULL);
    ui_main_menu();
    debugNetFinish();
}
