#ifndef REMAP_H
#define REMAP_H

#include <stdlib.h>
#include <stdbool.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <math.h>
#include <string.h>
#include "debugnet.h"

#define debug(format, args...) debugNetPrintf(DEBUG, format "\n", args)

typedef enum trigger {
    CTRL_SELECT      = 0x000001,        //!< Select button.
    CTRL_L3          = 0x000002,        //!< L3 button.
    CTRL_R3          = 0x000004,        //!< R3 button.
    CTRL_START       = 0x000008,        //!< Start button.
    CTRL_UP          = 0x000010,        //!< Up D-Pad button.
    CTRL_RIGHT       = 0x000020,        //!< Right D-Pad button.
    CTRL_DOWN        = 0x000040,        //!< Down D-Pad button.
    CTRL_LEFT        = 0x000080,        //!< Left D-Pad button.
    CTRL_LTRIGGER    = 0x000100,        //!< Left trigger.
    CTRL_RTRIGGER    = 0x000200,        //!< Right trigger.
    CTRL_L1          = 0x000400,        //!< L1 button.
    CTRL_R1          = 0x000800,        //!< R1 button.
    CTRL_TRIANGLE    = 0x001000,        //!< Triangle button.
    CTRL_CIRCLE      = 0x002000,        //!< Circle button.
    CTRL_CROSS       = 0x004000,        //!< Cross button.
    CTRL_SQUARE      = 0x008000,        //!< Square button.

    RIGHT_TRIGGER = 0x20000,
    LEFT_TRIGGER,

    RS_ANY = 0x300000,
    LS_ANY,

    TOUCHSCREEN_NW = 0x400000,
    TOUCHSCREEN_NE,
    TOUCHSCREEN_SW,
    TOUCHSCREEN_SE,

    RS_UP = 0x500000,
    RS_DOWN,
    RS_LEFT,
    RS_RIGHT,

    LS_UP,
    LS_DOWN,
    LS_LEFT,
    LS_RIGHT,

} trigger_t;

extern int TRIGGERS[30];
#define TRIGGERS_LAST_TRANSIENT LS_RIGHT
#define TRIGGERS_LAST_BUTTON CTRL_SQUARE
#define TRIGGERS_COUNT 30
#define TRIGGERS_BUTTONS_COUNT 14
#define TRIGGERS_TRANSIENT_COUNT TRIGGERS_BUTTONS_COUNT + 10

typedef enum {
    ACTION_BUTTON,
    ACTION_FRONTTOUCHSCREEN,
    ACTION_BACKTOUCHSCREEN,
    ACTION_RS,
    ACTION_LS,
    ACTION_TRIGGER,
} action_type_t;

typedef struct {
    action_type_t type;
    int value;
    short x, y;
} action_t;

typedef struct {
    char size;
    char index;
    action_t *list;
} action_list_t;

typedef struct {
    short back_touch_deadzone_vertical, back_touch_deadzone_horizontal;
    short front_touch_deadzone_vertical, front_touch_deadzone_horizontal;

    unsigned char rs_deadzone, ls_deadzone;
    unsigned char triggers_deadzone;

    bool disable_display;

    unsigned char size;
    trigger_t *triggers;
    action_list_t *actions;
} remap_config_t;

#define REMAP_MAX_ACTIONS 24

#define STICK_ZERO 255 / 2
action_t make_button_action(int identifier);
action_t make_touch_action(int x, int y, int port);
action_t make_stick_action(int x, int y, int type);
action_t make_trigger_action(int identifier);

//

void remap_deadzone_ignore(remap_config_t config, SceCtrlData *mut_pad, SceTouchData *mut_front, SceTouchData *mut_back);
void remap(remap_config_t config, SceCtrlData *mut_pad, SceTouchData *mut_front, SceTouchData *mut_back);

int remap_read_trigger(trigger_t *trigger, SceCtrlData pad, SceTouchData front, SceTouchData back);
int remap_read_actions(action_list_t *actions, SceCtrlData pad, SceTouchData front, SceTouchData back);

#define TRIGGER_NAME_SIZE 256
#define ACTION_NAME_SIZE 1024
void remap_config_trigger_title(int id, char buf[TRIGGER_NAME_SIZE]);
bool remap_config_action_title(action_t action, char name[256]);
void remap_config_pair_title(remap_config_t config, int n, char buf[ACTION_NAME_SIZE]);

#include "config.h"

#endif
