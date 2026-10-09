#include "UIManager.hpp"

namespace ui {

    void ScreenManager::init() {
        lv_display_t* display = lv_display_get_default();
        lv_theme_t* theme = lv_theme_mono_init(display, true, LV_FONT_DEFAULT);
#if LV_USE_KEYBOARD
        static lv_theme_t* keyboard_theme = nullptr;
        static lv_style_t keyboard_border_style;
        static lv_style_t compact_padding_style;
        static lv_style_t default_button_style;
        if (keyboard_theme == nullptr) {
            keyboard_theme = lv_theme_create();
            LV_ASSERT_MALLOC(keyboard_theme);
            lv_style_init(&compact_padding_style);
            lv_style_set_pad_all(&compact_padding_style, 0);
            lv_style_set_pad_gap(&compact_padding_style, 0);
            lv_style_init(&default_button_style);
            lv_style_set_height(&default_button_style, 12);
            lv_style_init(&keyboard_border_style);
            lv_style_set_border_width(&keyboard_border_style, 1);
            lv_style_set_border_color(&keyboard_border_style, lv_color_white());
            lv_style_set_border_opa(&keyboard_border_style, LV_OPA_COVER);
            lv_style_set_border_side(&keyboard_border_style, LV_BORDER_SIDE_FULL);
            lv_style_set_radius(&keyboard_border_style, 0);
        }
        lv_theme_copy(keyboard_theme, theme);
        lv_theme_set_parent(keyboard_theme, theme);
        lv_theme_set_apply_cb(keyboard_theme, [](lv_theme_t*, lv_obj_t* obj) {
            lv_obj_add_style(obj, &compact_padding_style, LV_PART_MAIN);
            if (lv_obj_check_type(obj, &lv_button_class)) {
                lv_obj_add_style(obj, &default_button_style, LV_PART_MAIN);
            }
            if (lv_obj_check_type(obj, &lv_keyboard_class)) {
                lv_obj_add_style(obj, &keyboard_border_style, LV_PART_ITEMS);
                lv_obj_add_style(obj, &keyboard_border_style, LV_PART_ITEMS | LV_STATE_PRESSED);
                lv_obj_add_style(obj, &keyboard_border_style, LV_PART_ITEMS | LV_STATE_CHECKED);
                lv_obj_add_style(obj, &keyboard_border_style, LV_PART_ITEMS | LV_STATE_DISABLED);
            }
        });
        theme = keyboard_theme;
#endif
        lv_display_set_theme(display, theme);
    }

    void ScreenManager::set_screen(Screen* screen) {
        if (current_screen) {
            current_screen->on_hide();
        }
        current_screen = screen;

        if (current_screen) {
            if(current_screen->root == nullptr)
                current_screen->build();
            lv_scr_load(current_screen->root);
            current_screen->on_show();
        } else {
            lv_scr_load(nullptr); // what happens here?
        }
    }

}
