#include "remap.h"

#include "../guilib/lib.h"

// trigger
enum TriggerType {
    TRIGGER_TYPE_BUTTON,
    TRIGGER_TYPE_TOUCH,
    TRIGGER_TYPE_STICK
};

int trigger_type(trigger_t trigger) {
    if (trigger <= CTRL_SQUARE) {
        return TRIGGER_TYPE_BUTTON;
    } else if (trigger <= TOUCHSCREEN_SE) {
        return TRIGGER_TYPE_TOUCH;
    } else if (trigger <= LS_RIGHT) {
        return TRIGGER_TYPE_STICK;
    }

    return -1;
}

// actionshortcuts
action_t make_button_action(int identifier) {
    action_t action = {
        .type = ACTION_BUTTON,
        .value = identifier,
    };

    return action;
}

action_t make_touch_action(int x, int y, int port) {
    if (port == SCE_TOUCH_PORT_FRONT) {
        action_t action = {
            .type = ACTION_FRONTTOUCHSCREEN,
            .x = x,
            .y = y,
        };
        return action;

    } else {
        action_t action = {
            .type = ACTION_BACKTOUCHSCREEN,
            .x = x,
            .y = y,
        };
        return action;
    }
}

// check
#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)
static int check_touch(SceTouchData scr, int lx, int ly, int rx, int ry) {
  for (int i = 0; i < scr.reportNum; i++) {
    int x = lerp(scr.report[i].x, 1919, WIDTH);
    int y = lerp(scr.report[i].y, 1087, HEIGHT);
    if (x < lx || x > rx || y < ly || y > ry) continue;
    return i;
  }

  return -1;
}

int trigger_check_touch(trigger_t trigger, SceTouchData data) {
    switch (trigger) {
        case TOUCHSCREEN_NW:
            return check_touch(data, 0, 0, WIDTH / 2, HEIGHT / 2);
        case TOUCHSCREEN_NE:
            return check_touch(data, WIDTH / 2, 0, WIDTH, HEIGHT / 2);
        case TOUCHSCREEN_SW:
            return check_touch(data, 0, HEIGHT / 2, WIDTH / 2, HEIGHT);
        case TOUCHSCREEN_SE:
            return check_touch(data, WIDTH / 2, HEIGHT / 2, WIDTH, HEIGHT);
    }

    return -1;
}

#define STICK_ZERO 255 / 2

bool trigger_check_stick(trigger_t trigger, SceCtrlData data) {
    int deadzone = 70;

    switch (trigger) {
        case RS_UP:
            return data.ry < STICK_ZERO - deadzone;
        case RS_DOWN:
            return data.ry > STICK_ZERO + deadzone;
        case RS_RIGHT:
            return data.rx > STICK_ZERO + deadzone;
        case RS_LEFT:
            return data.rx < STICK_ZERO - deadzone;

        case LS_UP:
            return data.ly < STICK_ZERO - deadzone;
        case LS_DOWN:
            return data.ly > STICK_ZERO + deadzone;
        case LS_RIGHT:
            return data.lx > STICK_ZERO + deadzone;
        case LS_LEFT:
            return data.lx < STICK_ZERO - deadzone;
    }

    return false;
}

// cancel
void cancel_button(int identifier, SceCtrlData *mut_pad) {
    mut_pad->buttons ^= identifier;
}

void cancel_touch(int id, SceTouchData *mut_data) {
    mut_data->reportNum -= 1;

    for (; id <= mut_data->reportNum; id++) {
        mut_data->report[id] = mut_data->report[id+1];
    }
}

void cancel_stick(trigger_t trigger, SceCtrlData *mut_pad) {
    switch (trigger) {
        case LS_UP:
        case LS_DOWN:
            mut_pad->ly = STICK_ZERO;
            break;
        case LS_RIGHT:
        case LS_LEFT:
            mut_pad->lx = STICK_ZERO;
            break;
        case RS_UP:
        case RS_DOWN:
            mut_pad->ry = STICK_ZERO;
            break;
        case RS_RIGHT:
        case RS_LEFT:
            mut_pad->rx = STICK_ZERO;
            break;
        case RS_ANY:
            mut_pad->rx = STICK_ZERO;
            mut_pad->ry = STICK_ZERO;
            break;
        case LS_ANY:
            mut_pad->lx = STICK_ZERO;
            mut_pad->ly = STICK_ZERO;
            break;
    }
}

// spawn
static id = 0;
void spawn_touch(int x, int y, SceTouchData *mut_data) {
    if (mut_data->reportNum < 4) {
        struct SceTouchReport touch = {
            .x = x,
            .y = y,
            .force = 0,
            .id = id++,
        };

        mut_data->report[mut_data->reportNum] = touch;
        mut_data->reportNum += 1;
    }
}

// config
remap_config_t load_config() {
    int size = 1;
    trigger_t *triggers = malloc(sizeof(trigger_t) * size);
    action_list_t *actions = malloc(sizeof(action_list_t) * size);

    /*
    triggers[0] = TOUCHSCREEN_NW;
    triggers[2] = TOUCHSCREEN_NE;

    actions[0] = alloc_single_action(make_button_action(SCE_CTRL_LTRIGGER));
    actions[3] = alloc_single_action(make_button_action(SCE_CTRL_RTRIGGER));
    */
    triggers[0] = CTRL_CROSS;

    action_t *cross_actions = malloc(sizeof(action_t) * 3);
    cross_actions[0] = make_button_action(SCE_CTRL_CIRCLE);
    cross_actions[1] = make_button_action(SCE_CTRL_SQUARE);
    cross_actions[2] = make_button_action(SCE_CTRL_TRIANGLE);
    action_list_t cross_actions_list = {
        .size = 3,
        .list = cross_actions,
    };

    actions[0] = cross_actions_list;

    remap_config_t config = {
        .size = size,
        .ls_deadzone = 70,
        .rs_deadzone = 70,
        .front_touch_deadzone_vertical = 100,
        .front_touch_deadzone_horizontal = 100,

        .back_touch_deadzone_vertical = 45,
        .back_touch_deadzone_horizontal = 200,
        .triggers = triggers,
        .actions = actions,
    };

    return config;
}

// deadzone
void deadzone_ignore_touch(int horizontal, int vertical, SceTouchData *data) {
    int cancel_ids_count = 0;
    int cancel_ids[8];

    for (int i = 0; i < data->reportNum; i++) {
        int x = lerp(data->report[i].x, 1919, WIDTH);
        int y = lerp(data->report[i].y, 1087, HEIGHT);

        if (x < horizontal || x > WIDTH - horizontal ||
            y < vertical || y > HEIGHT - vertical) {
            cancel_ids[cancel_ids_count] = i;
            cancel_ids_count++;
        }
    }

    for (int i = cancel_ids_count - 1; i >= 0; i--) {
        cancel_touch(cancel_ids[i], data);
    }
}

void deadzone_ignore(remap_config_t config, SceCtrlData *mut_pad, SceTouchData *mut_front, SceTouchData *mut_back) {
    if (abs(mut_pad->rx - STICK_ZERO) < config.rs_deadzone && abs(mut_pad->ry - STICK_ZERO) < config.rs_deadzone ) {
        cancel_stick(RS_ANY, mut_pad);
    }

    if (abs(mut_pad->lx - STICK_ZERO) < config.ls_deadzone && abs(mut_pad->ly - STICK_ZERO) < config.ls_deadzone ) {
        cancel_stick(LS_ANY, mut_pad);
    }

    deadzone_ignore_touch(config.front_touch_deadzone_horizontal, config.front_touch_deadzone_vertical, mut_front);
    deadzone_ignore_touch(config.back_touch_deadzone_horizontal, config.back_touch_deadzone_vertical, mut_back);
}

// remap
void remap(remap_config_t config, SceCtrlData *mut_pad, SceTouchData *mut_front, SceTouchData *mut_back) {
    deadzone_ignore(config, mut_pad, mut_front, mut_back);

    SceCtrlData pad;
    memcpy(&pad, mut_pad, sizeof(pad));

    for (int i = 0; i < config.size; i++) {
        trigger_t trigger = config.triggers[i];

        bool firing = false;
        int identifier = -1;

        int type = trigger_type(trigger);
        switch (type) {
            case TRIGGER_TYPE_BUTTON:
                firing = pad.buttons & trigger;
                break;
            case TRIGGER_TYPE_TOUCH:
                identifier = trigger_check_touch(trigger, *mut_back);
                firing = identifier != -1;
                break;
            case TRIGGER_TYPE_STICK:
                firing = trigger_check_stick(trigger, pad);
                break;
        }

        if (firing) {
            // prevent
            switch (type) {
                case TRIGGER_TYPE_BUTTON:
                    cancel_button((int) trigger, mut_pad);
                    break;
                case TRIGGER_TYPE_TOUCH:
                    cancel_touch(identifier, mut_back);
                    break;
                case TRIGGER_TYPE_STICK:
                    cancel_stick(trigger, mut_pad);
                    break;
            }

            action_list_t array = config.actions[i];
            for (int n = 0; n < array.size; n++) {
                action_t action = array.list[n];

                switch (action.type) {
                    case ACTION_BUTTON:
                        mut_pad->buttons |= action.value;
                        break;
                    case ACTION_FRONTTOUCHSCREEN:
                    case ACTION_BACKTOUCHSCREEN:
                        spawn_touch(action.x, action.y, action.type == ACTION_FRONTTOUCHSCREEN ? mut_front : mut_back);
                        break;
                    case ACTION_RS:
                    case ACTION_LS:
                        break;
                }

            }
        }
    }
}
