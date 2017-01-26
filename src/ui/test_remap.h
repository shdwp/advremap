#include <psp2/ctrl.h>
#include <vita2d.h>

#include "../remap/remap.h"

int ui_test_remap(remap_config_t conf);
void draw_touch_at(SceTouchData touch, int x, int y);
void draw_sticks_at(SceCtrlData pad, int x, int y);
