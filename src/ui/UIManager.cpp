#include "UIManager.hpp"

namespace ui {

    void ScreenManager::init() {
        lv_display_t* display = lv_display_get_default();
        lv_theme_t* theme = lv_theme_mono_init(display, true, LV_FONT_DEFAULT);
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
