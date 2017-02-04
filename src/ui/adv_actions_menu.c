#include "main_menu.h"

#define MENU_RELOAD 2

#define FRONT_TOUCHSCREEN 0x600000
#define BACK_TOUCHSCREEN 0x600001

#define ACTION_VARIANTS_SIZE 24
static int action_variants[] = {
    CTRL_SELECT,
    CTRL_START,
    CTRL_UP,
    CTRL_RIGHT,
    CTRL_DOWN,
    CTRL_LEFT,
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

    LEFT_TRIGGER,
    RIGHT_TRIGGER,

    FRONT_TOUCHSCREEN,
    BACK_TOUCHSCREEN,
};

int ui_new_action_menu_loop(int cursor_id, void *context) {
    if (was_button_pressed(SCE_CTRL_CROSS)) {
        action_t new_action;
        switch (cursor_id) {
            case FRONT_TOUCHSCREEN:
            case BACK_TOUCHSCREEN:
                if (true) {
                    int x, y;
                    char coordinates[1024];
                    ime_dialog(coordinates, "Enter X,Y (separated by ,)", "0,0");
                    sscanf(coordinates, "%d,%d", &x, &y);

                    x = ((float) x / WIDTH) * 1919;
                    y = ((float) y / HEIGHT) * 1087;
                    new_action = make_touch_action(x, y, cursor_id == FRONT_TOUCHSCREEN ? SCE_TOUCH_PORT_FRONT : SCE_TOUCH_PORT_BACK);
                } break;
            case RS_UP:
            case RS_DOWN:
            case RS_LEFT:
            case RS_RIGHT:
                new_action = make_stick_action(
                        cursor_id == RS_LEFT ? 0 :
                        cursor_id == RS_RIGHT ? 255 :
                        STICK_ZERO,
                        cursor_id == RS_UP ? 0 :
                        cursor_id == RS_DOWN ? 255 :
                        STICK_ZERO,
                        ACTION_RS);
                break;
            case LS_UP:
            case LS_DOWN:
            case LS_LEFT:
            case LS_RIGHT:
                new_action = make_stick_action(
                        cursor_id == LS_LEFT ? 0 :
                        cursor_id == LS_RIGHT ? 255 :
                        STICK_ZERO,
                        cursor_id == LS_UP ? 0 :
                        cursor_id == LS_DOWN ? 255 :
                        STICK_ZERO,
                        ACTION_LS);
                break;
            case RIGHT_TRIGGER:
            case LEFT_TRIGGER:
                new_action = make_trigger_action(cursor_id);
                break;
            default:
                new_action = make_button_action(cursor_id);
                break;
        }

        action_list_t *actions = (action_list_t *) context;
        if (actions->index == -1) {
            actions->list = realloc(actions->list, sizeof(action_t) * (actions->size + 1));
            actions->list[actions->size++] = new_action;
        } else {
            actions->list[actions->index] = new_action;
        }

        return GUI_EXIT;
    }

    return GUI_CONTINUE;
}

int ui_new_action_menu(action_list_t *actions) {
    int idx = 0;
    struct menu_entry menu[ACTION_VARIANTS_SIZE + 3];

    char names[ACTION_VARIANTS_SIZE][TRIGGER_NAME_SIZE];
    for (int i = 0; i < ACTION_VARIANTS_SIZE; i++) {
        int variant = action_variants[i];
        action_t action = {
            .type = -1,
            .value = variant,
        };

        if (variant <= TRIGGERS_LAST_TRANSIENT) {
            action.type = ACTION_BUTTON;
        } else if (variant == FRONT_TOUCHSCREEN) {
            action.type = ACTION_FRONTTOUCHSCREEN;
        } else if (variant == BACK_TOUCHSCREEN) {
            action.type = ACTION_BACKTOUCHSCREEN;
        } else if (variant == RIGHT_TRIGGER || variant == LEFT_TRIGGER) {
            action.type = ACTION_TRIGGER;
        } else if (RS_UP <= variant <= RS_RIGHT) {
            action.type = ACTION_RS;
        } else if (LS_UP <= variant <= LS_RIGHT) {
            action.type = ACTION_LS;
        }

        remap_config_action_title(action, names[i]);
        menu[idx++] = (struct menu_entry) { .name = names[i], .id = action_variants[i], };
    }

    struct menu_geom geom = make_geom_centered(200, 400);
    return display_menu(menu, idx, &geom, &ui_new_action_menu_loop, NULL, NULL, DEFAULT_GUIDE, actions);
}

int ui_adv_actions_menu_loop(int cursor_id, void *context) {
    action_list_t *actions = (action_list_t *) context;

    if (was_button_pressed(SCE_CTRL_SQUARE)) {
        for (int i = 0, n = 0; i < actions->size; i++) {
            if (cursor_id == i) {
                continue;
            } else {
                actions->list[n++] = actions->list[i];
            }
        }

        actions->size -= 1;
        actions->list = realloc(actions->list, sizeof(action_t) * actions->size);

        return MENU_RELOAD;
    }

    if (was_button_pressed(SCE_CTRL_TRIANGLE)) {
        actions->index = -1;
        ui_new_action_menu(actions);

        return MENU_RELOAD;
    }

    if (was_button_pressed(SCE_CTRL_CROSS)) {
        actions->index = cursor_id;
        ui_new_action_menu(actions);

        return MENU_RELOAD;
    }

    return 0;
}

int ui_adv_actions_menu(action_list_t *actions) {
    int ret;
    do {
        struct menu_entry menu[3 + actions->size];
        int idx = 0;
        menu[idx++] = (struct menu_entry) { .name = "List of actions: ", .separator = true, .disabled = true, };

        char names[actions->size][TRIGGER_NAME_SIZE];
        for (int i = 0; i < actions->size; i++) {
            action_t action = actions->list[i];
            remap_config_action_title(actions->list[i], names[i]);
            menu[idx++] = (struct menu_entry) { .name = names[i], .id = i, };
        }

        struct menu_geom geom = make_geom_centered(400, 200);
        ret = display_menu(menu, idx, &geom, &ui_adv_actions_menu_loop, NULL, NULL, EXT_GUIDE(ICON_SQUARE("delete"), ICON_TRIANGLE("add")), actions);
    } while (ret != GUI_EXIT);

    return GUI_EXIT;
}
