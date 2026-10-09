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

        lv_group_t* focus_group() {
            if (focus_group_ == nullptr) {
                focus_group_ = lv_group_create();
            }
            return focus_group_;
        }

    protected:

        lv_obj_t* root = nullptr;

        void add_focusable(lv_obj_t* object, bool invert_on_focus = true) {
            static lv_style_t focus_style;
            static bool focus_style_initialized = false;
            if (!focus_style_initialized) {
                lv_style_init(&focus_style);
                lv_style_set_bg_opa(&focus_style, LV_OPA_COVER);
                lv_style_set_bg_color(&focus_style, lv_color_white());
                lv_style_set_text_color(&focus_style, lv_color_black());
                lv_style_set_border_color(&focus_style, lv_color_white());
                lv_style_set_outline_color(&focus_style, lv_color_white());
                lv_style_set_outline_opa(&focus_style, LV_OPA_COVER);
                lv_style_set_outline_width(&focus_style, 1);
                lv_style_set_outline_pad(&focus_style, 1);
                focus_style_initialized = true;
            }
            if (invert_on_focus) {
                lv_obj_add_flag(object, LV_OBJ_FLAG_STATE_TRICKLE);
                apply_focus_style(object, &focus_style);
            }
            lv_group_add_obj(focus_group(), object);
        }

    private:
        static void apply_focus_style(lv_obj_t* object, lv_style_t* style) {
            lv_obj_add_style(object, style, LV_STATE_FOCUSED);
            lv_obj_add_style(object, style, LV_STATE_FOCUS_KEY);
            for (uint32_t index = 0; index < lv_obj_get_child_count(object); ++index) {
                apply_focus_style(lv_obj_get_child(object, index), style);
            }
        }

        friend class ScreenManager;
        lv_group_t* focus_group_ = nullptr;
    };

}
