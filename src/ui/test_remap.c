#include "test_remap.h"

#include "../guilib/lib.h"
#include "../remap/remap.h"

static remap_config_t remap_config;

void draw_pad_at(SceCtrlData pad, int x, int y) {
    int r = WIDTH - WIDTH / 3;
    int l = WIDTH / 3;

    int ids[] = {
        SCE_CTRL_CROSS, SCE_CTRL_CIRCLE, SCE_CTRL_TRIANGLE, SCE_CTRL_SQUARE,
        SCE_CTRL_DOWN, SCE_CTRL_RIGHT, SCE_CTRL_UP, SCE_CTRL_LEFT,
        SCE_CTRL_LTRIGGER, SCE_CTRL_RTRIGGER, SCE_CTRL_SELECT, SCE_CTRL_START,
    };
    char *chars[] = {
        "x", "o", "t", "s",
        "D", "R", "U", "L",
        "L", "R", "S", "S",
    };
    int pos[][2] = {
        {r, 130}, {r+30, 100}, {r, 70}, {r-30, 100},
        {l, 130}, {l+30, 100}, {l, 70}, {l-30, 100},
        {l, 40}, {r, 40}, {r-30, 160}, {r, 160},
    };
    int len = 12;

    int active_color = 0xffff00ff, inactive_color = 0xaaffffff;

    for (int i = 0; i < len; i++) {
        int color = pad.buttons & ids[i] ? active_color : inactive_color;
        vita2d_pgf_draw_text(gui_font, pos[i][0]+x, pos[i][1]+y, color, 1.0f, chars[i]);
    }

    vita2d_pgf_draw_text(gui_font, l+x, 40+y, pad.lt > 0 ? active_color : inactive_color, 1.0f, "L");
    vita2d_pgf_draw_text(gui_font, r+x, 40+y, pad.rt > 0 ? active_color : inactive_color, 1.0f, "R");

    float stick_fac = 10;
    int rx = pad.rx;
    int ry = pad.ry;
    vita2d_draw_fill_circle(r - 100 + x + (float) rx / stick_fac, 130 + y + (float) ry / stick_fac, 10, inactive_color);

    int lx = pad.lx;
    int ly = pad.ly;
    vita2d_draw_fill_circle(l + 100 + x + (float) lx / stick_fac, 130 + y + (float) ly / stick_fac, 10, inactive_color);
}

void draw_touch_at(SceTouchData touch, int x, int y) {
    int width = WIDTH - x * 2;
    int height = 100;

    vita2d_draw_rectangle(x, y, width, height, 0x30ff00ff);
    for (int i = 0; i < touch.reportNum; i++) {
        int tx = lerp(touch.report[i].x, 1919, 960);
        int ty = lerp(touch.report[i].y, 1087, 544);

        float tpx = (float) tx / WIDTH;
        float tpy = (float) ty / HEIGHT;

        vita2d_draw_fill_circle(tpx * width + x, tpy * height + y, 10, 0xffffffff);

        char xyz[1024];
        sprintf(&xyz, "%d;%d", tx, ty);
        vita2d_pgf_draw_text(gui_font, tpx * width + x, tpy * height + y, 0xffffffff, 1.0f, xyz);
    }
}

void ui_test_remap_draw() {
    SceCtrlData pad;
    sceCtrlPeekBufferPositive(0, &pad, 1);

    SceTouchData front;
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &front, 1);
    SceTouchData back;
    sceTouchPeek(SCE_TOUCH_PORT_BACK, &back, 1);

    remap(remap_config, &pad, &front, &back);

    draw_touch_at(front, 300, 90);
    draw_touch_at(back, 300, 200);
    draw_pad_at(pad, 0, 300);
}

int ui_test_remap_loop(int cursor_id, void *context) {
  if (is_button_down(SCE_CTRL_START) && is_button_down(SCE_CTRL_SELECT)) {
      return GUI_EXIT;
  }

  return GUI_CONTINUE;
}

int ui_test_remap_back(void *context) {
  return GUI_CONTINUE;
}

int ui_test_remap() {
  sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG_WIDE);
  sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
  sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);
  sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);

  struct menu_entry menu[16];
  int idx = 0;

  menu[idx++] = (struct menu_entry) { .name = "To quit press Start+Select", .disabled = true };

  struct menu_geom geom = make_geom_centered(300, 36);
  geom.x = 50;
  geom.y = 50;
  geom.statusbar = false;
  return display_menu(menu, idx, &geom, &ui_test_remap_loop, &ui_test_remap_back, &ui_test_remap_draw, NULL);
}
