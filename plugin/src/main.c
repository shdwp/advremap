#include <taihen.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/apputil.h>
#include <psp2/io/dirent.h>
#include <psp2/sysmodule.h>
#include <psp2/rtc.h>

#include "../src/remap/remap.h"
#include "../src/remap/config.h"

#include "blit.h"

// meta
#define CtrlBufferWrapper(n) static int tai_wrapper_##n(int p, SceCtrlData *ctrl, int c) { return ctrl_buffer_wrapper(p, tai_hook[n], ctrl, c); }
#define CtrlTouchWrapper(n) static int tai_wrapper_##n(int p, SceTouchData *data, int c) { return ctrl_touch_wrapper(p, tai_hook[n], data, c); }
#define CtrlTouchRegionWrapper(n) static int tai_wrapper_##n(int p, SceTouchData *data, int c, int r) { return ctrl_touch_region_wrapper(p, tai_hook[n], data, c, r); }

#define HOOKS_COUNT 12
static SceUID g_hooks[HOOKS_COUNT];
static tai_hook_ref_t tai_hook[HOOKS_COUNT];
static char mem_config[CONFIG_MEM_MAX_SIZE] = "ADVREMAP_IM_A_LOUSY_PROGRAMMER";

CtrlBufferWrapper(0)
CtrlBufferWrapper(1)
CtrlBufferWrapper(2)
CtrlBufferWrapper(3)

CtrlTouchWrapper(4)
CtrlTouchWrapper(5)
CtrlTouchRegionWrapper(6)
CtrlTouchRegionWrapper(7)

static int tai_wrapper_8(const SceDisplayFrameBuf *pParam, int sync) { return framebuf_set(tai_hook[6], pParam, sync); }

CtrlBufferWrapper(9)
CtrlBufferWrapper(10)
CtrlBufferWrapper(11)

// remap
static int display_notice_iterations;
static remap_config_t remap_config;

int ctrl_buffer_wrapper(int port, tai_hook_ref_t ref_hook, SceCtrlData *ctrl, int count) {
  if (ref_hook == 0) {
    return 1;
  } else {
    int ret = TAI_CONTINUE(int, ref_hook, port, ctrl, count);

    SceTouchData front, back;
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &front, 1);
    sceTouchPeek(SCE_TOUCH_PORT_BACK, &back, 1);

    remap(remap_config, ctrl, &front, &back);

    return ret;
  }
}

int ctrl_touch_wrapper(int port, tai_hook_ref_t ref_hook, SceTouchData *data, int count) {
  if (ref_hook == 0) {
    return 1;
  } else {
    int ret = TAI_CONTINUE(int, ref_hook, port, data, count);

    SceCtrlData ctrl;
    sceCtrlPeekBufferPositive(0, &ctrl, 1);

    if (port == SCE_TOUCH_PORT_FRONT) {
      SceTouchData back;
      sceTouchPeek(SCE_TOUCH_PORT_BACK, &back, 1);

      remap(remap_config, &ctrl, data, &back);
    } else {
      SceTouchData front;
      sceTouchPeek(SCE_TOUCH_PORT_FRONT, &front, 1);

      remap(remap_config, &ctrl, &front, data);
    }

    return ret;
  }
}

int ctrl_touch_region_wrapper(int port, tai_hook_ref_t ref_hook, SceTouchData *data, int count, int region) {
  if (ref_hook == 0) {
    return 1;
  } else {
    int ret = TAI_CONTINUE(int, ref_hook, port, data, count, region);

    SceCtrlData ctrl;
    sceCtrlPeekBufferPositive(0, &ctrl, 1);

    if (port == SCE_TOUCH_PORT_FRONT) {
      SceTouchData back;
      sceTouchPeek(SCE_TOUCH_PORT_BACK, &back, 1);

      remap(remap_config, &ctrl, data, &back);
    } else {
      SceTouchData front;
      sceTouchPeek(SCE_TOUCH_PORT_FRONT, &front, 1);

      remap(remap_config, &ctrl, &front, data);
    }

    return ret;
  }
}

int framebuf_set(tai_hook_ref_t ref_hook, const SceDisplayFrameBuf *pParam, int sync) {
  display_notice_iterations++;
  if (display_notice_iterations < 200) {
    blit_set_frame_buf(pParam);
    blit_set_color(0x00ff00ff, 0x00101010);
    blit_stringf(100, 100, ":: advremap loaded, config size %d", remap_config.size);
  }

  return TAI_CONTINUE(int, ref_hook, pParam, sync);
}

// start/stop
void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args) {
  config_mem_load((void *) mem_config, &remap_config);

  void *wrappers[HOOKS_COUNT] = {
    tai_wrapper_0,
    tai_wrapper_1,
    tai_wrapper_2,
    tai_wrapper_3,
    tai_wrapper_4,
    tai_wrapper_5,
    tai_wrapper_6,
    tai_wrapper_7,
    tai_wrapper_8,
    tai_wrapper_9,
    tai_wrapper_10,
    tai_wrapper_11,
  };

  int nids[HOOKS_COUNT] = {
      0xA9C3CED6, // sceCtrlPeekBufferPositive
      0x15F81E8C, // sceCtrlPeekBufferPositive2
      0x67E7AB83, // sceCtrlReadBufferPositive
      0xC4226A3E, // sceCtrlReadBufferPositive2
      0xFF082DF0, // sceCtrlTouchPeek
      0x169A1D58, // sceCtrlTouchRead
      0x4440622,  // sceTouchPeekRegion
      0x93B5635B, // sceTouchReadRegion
      // \/ this causing some games to CRASH
      0x7A410B64, // sceDisplaySetFrameBuf
      0xA59454D3, // sceCtrlPeekBufferPositiveExt
      0x860BF292, // sceCtrlPeekBufferPositiveExt2
      0xA7178860, // sceCtrlReadBufferPositiveExt2
  };

  for (int i = 0; i < HOOKS_COUNT; i++) {
    if (nids[i] == 0) {
      continue;
    }

    g_hooks[i] = taiHookFunctionImport(
        &tai_hook[i],
        TAI_MAIN_MODULE,
        TAI_ANY_LIBRARY,
        nids[i],
        wrappers[i]
        );
  }

  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
  for (int i = 0; i < HOOKS_COUNT; i++) {
    if (g_hooks[i] >= 0) taiHookRelease(g_hooks[i], tai_hook[i]);
  }

  return SCE_KERNEL_STOP_SUCCESS;
}
