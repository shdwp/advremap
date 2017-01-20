#include <taihen.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/kernel/modulemgr.h>

#include "../src/remap/remap.h"

// meta
#define HOOKS_COUNT 6
static SceUID g_hooks[HOOKS_COUNT];
static tai_hook_ref_t tai_hook[HOOKS_COUNT];
#define CtrlBufferWrapper(n) static int tai_wrapper_##n(int p, SceCtrlData *ctrl, int c) { return ctrl_buffer_wrapper(p, tai_hook[n], ctrl, c); }
#define CtrlTouchWrapper(n) static int tai_wrapper_##n(int p, SceTouchData *data, int c) { return ctrl_touch_wrapper(p, tai_hook[n], data, c); }
CtrlBufferWrapper(0)
CtrlBufferWrapper(1)
CtrlBufferWrapper(2)
CtrlBufferWrapper(3)
CtrlTouchWrapper(4)
CtrlTouchWrapper(5)
void _start() __attribute__ ((weak, alias ("module_start")));

static remap_config_t remap_config;
/*
   PCSB00866 - TOCS
   PCSB00867 - P4DAN
   PCSB00743 - ??
   PCSB00497 - YS
*/

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

int module_start(SceSize argc, const void *args) {
  remap_config = load_config();

  int i = -1;

  g_hooks[++i] = taiHookFunctionImport(&tai_hook[i],
      TAI_MAIN_MODULE,
      TAI_ANY_LIBRARY,
      0xA9C3CED6, // sceCtrlPeekBufferPositive
      tai_wrapper_0);

  g_hooks[++i] = taiHookFunctionImport(&tai_hook[i],
      TAI_MAIN_MODULE,
      TAI_ANY_LIBRARY,
      0x15F81E8C, // sceCtrlPeekBufferPositive2
      tai_wrapper_1);

  g_hooks[++i] = taiHookFunctionImport(&tai_hook[i],
      TAI_MAIN_MODULE,
      TAI_ANY_LIBRARY,
      0x67E7AB83, // sceCtrlReadBufferPositive
      tai_wrapper_2);

  g_hooks[++i] = taiHookFunctionImport(&tai_hook[i],
      TAI_MAIN_MODULE,
      TAI_ANY_LIBRARY,
      0xC4226A3E, // sceCtrlReadBufferPositive2
      tai_wrapper_3);

  g_hooks[++i] = taiHookFunctionImport(&tai_hook[i],
      TAI_MAIN_MODULE,
      TAI_ANY_LIBRARY,
      0xFF082DF0, // sceCtrlTouchPeek
      tai_wrapper_4);

  g_hooks[++i] = taiHookFunctionImport(&tai_hook[i],
      TAI_MAIN_MODULE,
      TAI_ANY_LIBRARY,
      0x169A1D58, // sceCtrlTouchRead
      tai_wrapper_5);

  return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
  for (int i = 0; i < HOOKS_COUNT; i++) {
    if (g_hooks[i] >= 0) taiHookRelease(g_hooks[i], tai_hook[i]);
  }

  return SCE_KERNEL_STOP_SUCCESS;
}
