#include "main_menu.h"

#include "test_remap.h"

#define MENU_RELOAD 2

#define FRONT_TOUCHSCREEN 0x600000
#define BACK_TOUCHSCREEN 0x600001

#define ACTION_VARIANTS_SIZE 21
static int action_variants[] = {
    CTRL_SELECT,
    CTRL_START,
    CTRL_UP,
    CTRL_RIGHT,
    CTRL_DOWN,
    CTRL_LEFT,
    CTRL_LTRIGGER,
    CTRL_RTRIGGER,
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

    FRONT_TOUCHSCREEN,
    BACK_TOUCHSCREEN,
};

int ui_new_action_menu_loop(int cursor_id, void *context) {
    if (was_button_pressed(SCE_CTRL_CROSS)) {
        action_t new_action;
        switch (cursor_id) {
            case FRONT_TOUCHSCREEN:
            case BACK_TOUCHSCREEN:
                new_action = make_touch_action(100, 100, cursor_id == FRONT_TOUCHSCREEN ? SCE_TOUCH_PORT_FRONT : SCE_TOUCH_PORT_BACK);
                break;

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

        if (variant <= LS_RIGHT) {
            remap_config_trigger_title(variant, names[i]);
        } else if (variant == FRONT_TOUCHSCREEN) {
            strcpy(names[i], "[  ]");
        } else if (variant == BACK_TOUCHSCREEN) {
            strcpy(names[i], "⊂⊃");
        }

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
                actions->list[i] = actions->list[n++];
            }

            actions->size -= 1;
            actions->list = realloc(actions->list, sizeof(action_t) * actions->size);
        }

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
