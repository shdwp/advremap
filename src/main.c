#include <psp2/ctrl.h>
#include <psp2/rtc.h>
#include <psp2/touch.h>

#include <vita2d.h>

#include "ui/main_menu.h"
#include "debugnet.h"

#include <psp2/apputil.h>
#include <psp2/io/dirent.h>

void main() {
    debugNetInit("192.168.12.223" , 18194, DEBUG);
    debugNetPrintf(DEBUG, "\n---------------------------------\n");

    SceAppUtilInitParam init;
    SceAppUtilBootParam boot;
    memset(&init, 0, sizeof(SceAppUtilInitParam));
    memset(&boot, 0, sizeof(SceAppUtilBootParam));

    sceAppUtilInit(&init, &boot);
    sceAppUtilPhotoMount();

    sceIoMkdir("ux0:data/advremap", 0777);
    sceIoMkdir("ux0:tai/advremap", 0777);

    sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG_WIDE);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);

    vita2d_init();
    vita2d_set_clear_color(RGBA8(0, 0, 0, 0));
    guilib_init(NULL, NULL);
    ui_main_menu();
    debugNetFinish();
}
