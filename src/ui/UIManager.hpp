#pragma once

#include <lvgl.h>

namespace ui {

    class Screen;

    inline void apply_button_border(lv_obj_t* button) {
        static lv_style_t border_style;
        static bool initialized = false;
        if (!initialized) {
            lv_style_init(&border_style);
            lv_style_set_border_width(&border_style, 1);
            lv_style_set_border_color(&border_style, lv_color_white());
            initialized = true;
        }
        lv_obj_add_style(button, &border_style, 0);
    }

    class ScreenManager {
    public:
        void init();
        void set_screen(Screen* screen);
        Screen* get_current_screen() { return current_screen; }

        static ScreenManager& instance() {
            static ScreenManager _instance;
            return _instance;
        }

    private:
        Screen* current_screen = nullptr;
    };

    class Screen {
    public:
        virtual ~Screen() {};

        virtual void on_show() {};
        virtual void on_hide() {};

        virtual void build() {};
    protected:

        lv_obj_t* root = nullptr;

    private:
        friend class ScreenManager;
    };

}
