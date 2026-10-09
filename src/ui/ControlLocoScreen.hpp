#pragma once

#include "UIManager.hpp"
#include "../AppState.h"

namespace ui {

    class ControlLocoScreen : public Screen {
    public:
        explicit ControlLocoScreen(Screen* back_screen)
            : back_screen_(back_screen) {}

        ~ControlLocoScreen() override {
            stop_refresh_timer();
        }

        void build() override {
            root = lv_obj_create(nullptr);
            lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_pad_all(root, 0, LV_PART_MAIN);
            lv_obj_set_style_border_width(root, 0, LV_PART_MAIN);
            lv_obj_set_layout(root, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                root,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START);

            lv_obj_t* header = lv_obj_create(root);
            lv_obj_set_size(header, 128, 12);
            lv_obj_set_style_pad_all(header, 0, LV_PART_MAIN);
            lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
            lv_obj_set_layout(header, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);

            lv_obj_t* back_button = create_button(header, "Back", 34, back_event_callback);
            LV_UNUSED(back_button);
            select_button_ = create_button(header, "Loco --", 94, select_event_callback);
            lv_obj_set_height(select_button_, 12);

            lv_obj_t* readout = lv_obj_create(root);
            lv_obj_set_size(readout, 128, 20);
            lv_obj_set_style_pad_all(readout, 0, LV_PART_MAIN);
            lv_obj_set_style_border_width(readout, 0, LV_PART_MAIN);
            lv_obj_set_layout(readout, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(readout, LV_FLEX_FLOW_ROW);

            speed_label_ = lv_label_create(readout);
            lv_label_set_text(speed_label_, "SPEED --%");
            lv_obj_set_width(speed_label_, 64);
            lv_obj_set_height(speed_label_, 20);
            lv_obj_set_style_pad_left(speed_label_, 2, LV_PART_MAIN);
            lv_obj_set_style_pad_top(speed_label_, 6, LV_PART_MAIN);
            lv_obj_set_style_border_width(speed_label_, 1, LV_PART_MAIN);

            direction_label_ = lv_label_create(readout);
            lv_label_set_text(direction_label_, "DIR --");
            lv_obj_set_width(direction_label_, 64);
            lv_obj_set_height(direction_label_, 20);
            lv_obj_set_style_pad_left(direction_label_, 2, LV_PART_MAIN);
            lv_obj_set_style_pad_top(direction_label_, 6, LV_PART_MAIN);
            lv_obj_set_style_border_width(direction_label_, 1, LV_PART_MAIN);

            lv_obj_t* functions = lv_obj_create(root);
            lv_obj_set_size(functions, 128, 24);
            lv_obj_add_flag(functions, LV_OBJ_FLAG_IGNORE_LAYOUT);
            lv_obj_set_pos(functions, 0, 32);
            lv_obj_set_style_pad_all(functions, 0, LV_PART_MAIN);
            lv_obj_set_style_border_width(functions, 0, LV_PART_MAIN);
            lv_obj_set_layout(functions, LV_LAYOUT_NONE);

            for (uint32_t index = 0; index < 16; ++index) {
                char text[5];
                if (index < 10) {
                    lv_snprintf(text, sizeof(text), "F%u", static_cast<unsigned>(index));
                } else {
                    lv_snprintf(text, sizeof(text), "%u", static_cast<unsigned>(index));
                }
                lv_obj_t* indicator = lv_obj_create(functions);
                lv_obj_set_size(indicator, 16, 12);
                lv_obj_set_pos(indicator, (index % 8) * 16, (index / 8) * 12);
                lv_obj_set_style_pad_all(indicator, 0, LV_PART_MAIN);
                lv_obj_set_style_border_width(indicator, 0, LV_PART_MAIN);
                lv_obj_set_style_border_color(indicator, lv_color_white(), LV_PART_MAIN);
                lv_obj_set_style_border_opa(indicator, LV_OPA_COVER, LV_PART_MAIN);
                lv_obj_set_style_bg_opa(indicator, LV_OPA_TRANSP, LV_PART_MAIN);
                lv_obj_t* label = lv_label_create(indicator);
                lv_label_set_text(label, text);
                lv_obj_set_size(label, 14, 10);
                lv_obj_set_pos(label, 1, 1);
                lv_obj_set_style_pad_all(label, 0, LV_PART_MAIN);
                lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);
                lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
                lv_obj_set_style_border_width(label, 0, LV_PART_MAIN);
                lv_obj_set_style_border_color(label, lv_color_white(), LV_PART_MAIN);
                lv_obj_set_style_border_opa(label, LV_OPA_COVER, LV_PART_MAIN);
                lv_obj_center(label);
                function_labels_[index] = label;
            }
        }

        void on_show() override {
            address_label_initialized_ = false;
            refresh_loco();
            if (refresh_timer_ == nullptr) {
                refresh_timer_ = lv_timer_create(refresh_timer_callback, refresh_period_ms, this);
            }
        }

        void on_hide() override {
            stop_refresh_timer();
        }

    private:
        static constexpr uint32_t refresh_period_ms = 100;

        Screen* back_screen_ = nullptr;
        lv_obj_t* select_button_ = nullptr;
        lv_obj_t* select_button_label_ = nullptr;
        lv_obj_t* speed_label_ = nullptr;
        lv_obj_t* direction_label_ = nullptr;
        lv_obj_t* function_labels_[16] = {};
        lv_timer_t* refresh_timer_ = nullptr;
        uint16_t displayed_addr_ = 0;
        bool address_label_initialized_ = false;

        using button_callback_t = void (*)(lv_event_t*);

        lv_obj_t* create_button(
            lv_obj_t* parent,
            const char* text,
            int32_t width,
            button_callback_t callback
        ) {
            lv_obj_t* button = lv_button_create(parent);
            apply_button_border(button);
            lv_obj_set_width(button, width);
            lv_obj_t* label = lv_label_create(button);
            lv_label_set_text(label, text);
            lv_obj_center(label);
            if (callback == select_event_callback) {
                select_button_label_ = label;
            }
            add_focusable(button);
            lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, this);
            return button;
        }

        static void refresh_timer_callback(lv_timer_t* timer) {
            auto* screen = static_cast<ControlLocoScreen*>(lv_timer_get_user_data(timer));
            screen->refresh_loco();
        }

        void refresh_loco() {
            const Loco& loco = AppState::get_instance().loco;
            if (!address_label_initialized_ || displayed_addr_ != loco.addr) {
                if (loco.addr == 0) {
                    lv_label_set_text(select_button_label_, "Loco --");
                } else {
                    lv_label_set_text_fmt(select_button_label_, "Loco %u", static_cast<unsigned>(loco.addr));
                }
                displayed_addr_ = loco.addr;
                address_label_initialized_ = true;
            }

            lv_label_set_text_fmt(speed_label_, "SPD %u%%", static_cast<unsigned>(loco.speed));
            switch (loco.dir) {
                case ThrottleDirection::Forward:
                    lv_label_set_text(direction_label_, "DIR FWD");
                    break;
                case ThrottleDirection::Reverse:
                    lv_label_set_text(direction_label_, "DIR REV");
                    break;
                case ThrottleDirection::Neutral:
                default:
                    lv_label_set_text(direction_label_, "DIR N");
                    break;
            }

            for (uint32_t index = 0; index < 16; ++index) {
                const bool active = loco.fns[index];
                lv_obj_set_style_border_width(function_labels_[index], active ? 1 : 0, LV_PART_MAIN);
            }
        }

        void stop_refresh_timer() {
            if (refresh_timer_ != nullptr) {
                lv_timer_del(refresh_timer_);
                refresh_timer_ = nullptr;
            }
        }

        static void back_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<ControlLocoScreen*>(lv_event_get_user_data(event));
            ScreenManager::instance().set_screen(screen->back_screen_);
        }

        static void select_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<ControlLocoScreen*>(lv_event_get_user_data(event));
            lv_label_set_text(screen->select_button_label_, "No roster loaded");
        }
    };

}
