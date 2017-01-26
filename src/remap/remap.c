#include "remap.h"

#include "../guilib/lib.h"

int TRIGGERS[] =  {
    CTRL_SELECT,
    CTRL_L3,
    CTRL_R3,
    CTRL_START,
    CTRL_UP,
    CTRL_RIGHT,
    CTRL_DOWN,
    CTRL_LEFT,
    CTRL_LTRIGGER,
    CTRL_RTRIGGER,
    CTRL_L1,
    CTRL_R1,
    CTRL_TRIANGLE,
    CTRL_CIRCLE,
    CTRL_CROSS,
    CTRL_SQUARE,

    RS_UP,
    RS_DOWN,
    RS_LEFT,
    RS_RIGHT,

    LS_UP,
    LS_DOWN,
    LS_LEFT,
    LS_RIGHT,

    RS_ANY,
    LS_ANY,

    TOUCHSCREEN_NW,
    TOUCHSCREEN_NE,
    TOUCHSCREEN_SW,
    TOUCHSCREEN_SE,
};

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
    return (action_t) {
        .type = ACTION_BUTTON,
        .value = identifier,
    };
}

action_t make_touch_action(int x, int y, int port) {
    return (action_t) {
        .type = port == SCE_TOUCH_PORT_FRONT ? ACTION_FRONTTOUCHSCREEN : ACTION_BACKTOUCHSCREEN,
        .x = x,
        .y = y,
    };
}

action_t make_stick_action(int x, int y, int type) {
    return (action_t) {
        .type = type,
        .x = x,
        .y = y
    };
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
#define TRIGGER_CHECK_STICK_DEADZONE 70

bool trigger_check_stick(trigger_t trigger, SceCtrlData data) {
    int deadzone = TRIGGER_CHECK_STICK_DEADZONE;

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
static int id = 0;
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

int remap_test_trigger(trigger_t trigger, SceCtrlData pad, SceTouchData front, SceTouchData back) {
    int type = trigger_type(trigger);
    switch (type) {
        case TRIGGER_TYPE_BUTTON:
            return pad.buttons & trigger ? 0 : -1;
        case TRIGGER_TYPE_TOUCH:
            return trigger_check_touch(trigger, back);
        case TRIGGER_TYPE_STICK:
            return trigger_check_stick(trigger, pad) ? 0 : -1;
        default:
            return -1;
    }
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

void remap_deadzone_ignore(remap_config_t config, SceCtrlData *mut_pad, SceTouchData *mut_front, SceTouchData *mut_back) {
    if (abs(mut_pad->rx - STICK_ZERO) < config.rs_deadzone && abs(mut_pad->ry - STICK_ZERO) < config.rs_deadzone ) {
        cancel_stick(RS_ANY, mut_pad);
    }

    if (abs(mut_pad->lx - STICK_ZERO) < config.ls_deadzone && abs(mut_pad->ly - STICK_ZERO) < config.ls_deadzone ) {
        cancel_stick(LS_ANY, mut_pad);
    }

    deadzone_ignore_touch(config.front_touch_deadzone_horizontal, config.front_touch_deadzone_vertical, mut_front);
    deadzone_ignore_touch(config.back_touch_deadzone_horizontal, config.back_touch_deadzone_vertical, mut_back);
}

int remap_read_actions(action_list_t *actions, SceCtrlData pad, SceTouchData front, SceTouchData back) {
    int actions_i = 0;

    for (int i = 0; i < TRIGGERS_NOTOUCH_COUNT; i++) {
        if (remap_test_trigger(TRIGGERS[i], pad, front, back) >= 0) {

            int type = trigger_type(TRIGGERS[i]);
            int stick_type = TRIGGERS[i] <= RS_RIGHT ? ACTION_RS : ACTION_LS;

            switch (type) {
                case TRIGGER_TYPE_BUTTON:
                    actions->list[actions_i++] = make_button_action(TRIGGERS[i]);
                    break;
                case TRIGGER_TYPE_STICK:
                    if (true) {
                        int x = stick_type == ACTION_RS ? pad.rx : pad.lx;
                        int y = stick_type == ACTION_RS ? pad.ry : pad.ly;

                        x -= STICK_ZERO;
                        y -= STICK_ZERO;

                        actions->list[actions_i++] = make_stick_action(
                                abs(x) > 15 ? x : 0,
                                abs(y) > 15 ? y : 0,
                                stick_type
                                );
                    }

                    break;
            }
        }
    }

    for (int i = 0; i < front.reportNum; i++) {
        actions->list[actions_i++] = make_touch_action(front.report[i].x, front.report[i].y, SCE_TOUCH_PORT_FRONT);
    }

    for (int i = 0; i < back.reportNum; i++) {
        actions->list[actions_i++] = make_touch_action(back.report[i].x, back.report[i].y, SCE_TOUCH_PORT_BACK);
    }

    actions->size = actions_i;
    actions->list = realloc(actions->list, sizeof(action_t) * actions_i);

    return actions->size > 0 ? 0 : -1;
}

int remap_read_trigger(trigger_t *trigger, SceCtrlData pad, SceTouchData front, SceTouchData back) {
    for (int i = 0; i < TRIGGERS_COUNT; i++) {
        if (remap_test_trigger(TRIGGERS[i], pad, front, back) >= 0) {
            *trigger = TRIGGERS[i];
            return 0;
        }
    }

    return -1;
}

void stick_values_append(char *x, char *y, int dx, int dy) {
    int overflow = *x;
    if (overflow + dx >= 255) {
        *x = 255;
    } else if (overflow + dx <= 0) {
        *x = 0;
    } else {
        *x += dx;
    }

    overflow = *y;
    if (overflow + dy > 255) {
        *y = 255;
    } else if (overflow - dy < 0) {
        *y = 0;
    } else {
        *y += dy;
    }
}

// remap
void remap(remap_config_t config, SceCtrlData *mut_pad, SceTouchData *mut_front, SceTouchData *mut_back) {
    remap_deadzone_ignore(config, mut_pad, mut_front, mut_back);

    SceCtrlData pad;
    memcpy(&pad, mut_pad, sizeof(pad));
    int spawned_buttons[TRIGGERS_BUTTONS_COUNT];
    int spawned_buttons_i = 0;

    for (int i = 0; i < config.size; i++) {
        trigger_t trigger = config.triggers[i];

        int type = trigger_type(trigger);
        int trigger_test = remap_test_trigger(trigger, pad, *mut_front, *mut_back);
        if (trigger_test >= 0) {
            // prevent
            bool should_prevent = true;
            for (int n = 0; n < spawned_buttons_i; n++) {
                if (spawned_buttons[n] == trigger) {
                    should_prevent = false;
                }
            }

            if (should_prevent) {
                switch (type) {
                    case TRIGGER_TYPE_BUTTON:
                        cancel_button((int) trigger, mut_pad);
                        break;
                    case TRIGGER_TYPE_TOUCH:
                        cancel_touch(trigger_test, mut_back);
                        break;
                    case TRIGGER_TYPE_STICK:
                        cancel_stick(trigger, mut_pad);
                        break;
                }
            }

            action_list_t array = config.actions[i];
            for (int n = 0; n < array.size; n++) {
                action_t action = array.list[n];
                int overflow = 0;

                switch (action.type) {
                    case ACTION_BUTTON:
                        mut_pad->buttons |= action.value;
                        spawned_buttons[spawned_buttons_i++] = action.value;
                        break;
                    case ACTION_FRONTTOUCHSCREEN:
                    case ACTION_BACKTOUCHSCREEN:
                        spawn_touch(action.x, action.y, action.type == ACTION_FRONTTOUCHSCREEN ? mut_front : mut_back);
                        break;
                    case ACTION_RS:
                        stick_values_append(&mut_pad->rx, &mut_pad->ry, action.x, action.y);
                        break;
                    case ACTION_LS:
                        stick_values_append(&mut_pad->lx, &mut_pad->ly, action.x, action.y);
                        break;
                }

            }
        }
    }
}

void remap_trigger_name(int id, char buf[TRIGGER_NAME_SIZE]) {
    char *trigger_name = NULL;
    switch (id) {
        case CTRL_SELECT: trigger_name = "Select"; break;
        case CTRL_START: trigger_name = "Start"; break;
        case CTRL_UP: trigger_name = "▲"; break;
        case CTRL_RIGHT: trigger_name = "▶"; break;
        case CTRL_DOWN: trigger_name = "▼"; break;
        case CTRL_LEFT: trigger_name = "◀"; break;
        case CTRL_LTRIGGER: trigger_name = "L◤"; break;
        case CTRL_RTRIGGER: trigger_name = "◥R"; break;
        case CTRL_TRIANGLE: trigger_name = "△"; break;
        case CTRL_CIRCLE: trigger_name = "◯"; break;
        case CTRL_CROSS: trigger_name = "╳"; break;
        case CTRL_SQUARE: trigger_name = "□"; break;

        case TOUCHSCREEN_NW: trigger_name = "⊂⊃ NW"; break;
        case TOUCHSCREEN_NE: trigger_name = "⊂⊃ NE"; break;
        case TOUCHSCREEN_SW: trigger_name = "⊂⊃ SW"; break;
        case TOUCHSCREEN_SE: trigger_name = "⊂⊃ SE"; break;
        case RS_UP: trigger_name = "RS▲"; break;
        case RS_DOWN: trigger_name = "RS▼"; break;
        case RS_LEFT: trigger_name = "RS◀"; break;
        case RS_RIGHT: trigger_name = "RS▶"; break;

        case LS_UP: trigger_name = "LS▲"; break;
        case LS_DOWN: trigger_name = "LS▼"; break;
        case LS_LEFT: trigger_name = "LS◀"; break;
        case LS_RIGHT: trigger_name = "LS▶"; break;
    }

    strcpy(buf, trigger_name);
}

void remap_config_action_name(remap_config_t config, int n, char buf[ACTION_NAME_SIZE]) {
    char trigger_name[256];
    remap_trigger_name(config.triggers[n], trigger_name);

    sprintf(buf, "%s -> ", trigger_name);
    for (int i = 0; i < config.actions[n].size; i++) {
        if (strlen(buf) > 768) {
            sprintf(buf, "%s (%d others)", buf, config.actions[n].size - i);
            break;
        }

        action_t action = config.actions[n].list[i];
        char name[256];
        bool name_set = false;
        switch (action.type) {
            case ACTION_BUTTON:
                name_set = true;
                remap_trigger_name(action.value, name);
                break;
            case ACTION_FRONTTOUCHSCREEN:
                name_set = true;
                strcpy(name, "[  ]");
                break;
            case ACTION_BACKTOUCHSCREEN:
                name_set = true;
                strcpy(name, "⊂⊃");
                break;
            case ACTION_RS:
                name_set = true;
                strcpy(name, "RS");
                break;
            case ACTION_LS:
                name_set = true;
                strcpy(name, "LS");
                break;
        }

        if (name_set) {
            sprintf(buf, "%s %s", buf, name);
        }
    }
}
