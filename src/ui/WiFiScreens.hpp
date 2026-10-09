#pragma once

#include "UIManager.hpp"
#include <WiFi.h>

namespace ui {

    class WiFiPasswordScreen : public Screen {
    public:
        explicit WiFiPasswordScreen(Screen* back_screen)
            : back_screen_(back_screen) {}

        void build() override {
            root = lv_obj_create(nullptr);
            lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_pad_all(root, 0, 0);
            lv_obj_set_layout(root, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                root,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START);

            password_textarea_ = lv_textarea_create(root);
            lv_obj_set_size(password_textarea_, 128, 18);
            lv_textarea_set_password_mode(password_textarea_, true);
            lv_textarea_set_one_line(password_textarea_, true);

            keyboard_ = lv_keyboard_create(root);
            lv_obj_set_size(keyboard_, 128, 46);
            lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_IGNORE_LAYOUT);
            lv_obj_align(keyboard_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
            lv_obj_set_style_pad_all(keyboard_, 0, 0);
            lv_obj_set_style_pad_gap(keyboard_, 0, 0);
            lv_obj_set_style_pad_all(keyboard_, 0, LV_PART_ITEMS);
            lv_keyboard_set_textarea(keyboard_, password_textarea_);
            lv_obj_add_event_cb(keyboard_, keyboard_event_callback, LV_EVENT_READY, this);
            lv_obj_add_event_cb(keyboard_, keyboard_event_callback, LV_EVENT_CANCEL, this);
        }

        void on_show() override {
            lv_textarea_set_text(password_textarea_, "");
            lv_textarea_set_cursor_pos(password_textarea_, LV_TEXTAREA_CURSOR_LAST);
            lv_group_focus_obj(password_textarea_);
        }

        void set_network_name(const char* network_name) {
            network_name_ = network_name;
        }

    private:
        Screen* back_screen_ = nullptr;
        lv_obj_t* password_textarea_ = nullptr;
        lv_obj_t* keyboard_ = nullptr;
        String network_name_;

        static void keyboard_event_callback(lv_event_t* event) {
            const lv_event_code_t code = lv_event_get_code(event);
            if (code != LV_EVENT_READY && code != LV_EVENT_CANCEL) {
                return;
            }

            auto* screen = static_cast<WiFiPasswordScreen*>(lv_event_get_user_data(event));
            if (code == LV_EVENT_READY) {
                WiFi.begin(
                    screen->network_name_.c_str(),
                    lv_textarea_get_text(screen->password_textarea_));
            }
            ScreenManager::instance().set_screen(screen->back_screen_);
        }
    };

    class WiFiSelectionScreen : public Screen {
    public:
        static constexpr uint32_t scan_timer_period_ms = 100;

        explicit WiFiSelectionScreen(Screen* back_screen)
            : back_screen_(back_screen), password_screen_(this) {}

        void build() override {
            root = lv_obj_create(nullptr);
            lv_obj_set_style_pad_all(root, 0, 0);
            lv_obj_set_layout(root, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                root,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START);

            back_button_ = lv_button_create(root);
            apply_button_border(back_button_);
            lv_obj_set_width(back_button_, 42);
            lv_obj_t* back_label = lv_label_create(back_button_);
            lv_label_set_text(back_label, "Back");
            lv_obj_center(back_label);
            lv_obj_add_event_cb(back_button_, back_button_event_callback, LV_EVENT_CLICKED, this);

            network_container_ = lv_obj_create(root);
            lv_obj_set_width(network_container_, 128);
            lv_obj_set_flex_grow(network_container_, 1);
            lv_obj_set_scroll_dir(network_container_, LV_DIR_VER);
            lv_obj_set_layout(network_container_, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(network_container_, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                network_container_,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START);
        }

        void on_show() override {
            start_scan();
        }

        void on_hide() override {
            stop_scan_timer();
            WiFi.scanDelete();
        }

    private:
        static constexpr int32_t network_row_height = 10;

        Screen* back_screen_ = nullptr;
        WiFiPasswordScreen password_screen_;
        lv_obj_t* back_button_ = nullptr;
        lv_obj_t* network_container_ = nullptr;
        lv_timer_t* scan_timer_ = nullptr;
        static void back_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<WiFiSelectionScreen*>(lv_event_get_user_data(event));
            ScreenManager::instance().set_screen(screen->back_screen_);
        }

        void start_scan() {
            clear_network_list();
            add_network_row("Scanning...");

            WiFi.scanDelete();
            const int scan_result = WiFi.scanNetworks(true, true);
            if (scan_result == WIFI_SCAN_RUNNING) {
                scan_timer_ = lv_timer_create(scan_timer_callback, scan_timer_period_ms, this);
                return;
            }

            finish_scan(scan_result);
        }

        static void scan_timer_callback(lv_timer_t* timer) {
            auto* screen = static_cast<WiFiSelectionScreen*>(lv_timer_get_user_data(timer));
            screen->poll_scan();
        }

        void poll_scan() {
            const int scan_result = WiFi.scanComplete();
            if (scan_result == WIFI_SCAN_RUNNING) {
                return;
            }

            stop_scan_timer();
            finish_scan(scan_result);
        }

        void finish_scan(int network_count) {
            clear_network_list();
            if (network_count <= 0) {
                add_network_row("No networks found");
                return;
            }

            for (int network_index = 0; network_index < network_count; ++network_index) {
                add_network_row(WiFi.SSID(network_index).c_str());
            }
            WiFi.scanDelete();
        }

        void clear_network_list() {
            lv_obj_clean(network_container_);
        }

        void stop_scan_timer() {
            if (scan_timer_ != nullptr) {
                lv_timer_del(scan_timer_);
                scan_timer_ = nullptr;
            }
        }

        void add_network_row(const char* network_name) {
            lv_obj_t* network_button = lv_btn_create(network_container_);
            apply_button_border(network_button);
            lv_obj_set_size(network_button, 128, network_row_height);
            lv_obj_t* network_label = lv_label_create(network_button);
            lv_label_set_text(network_label, network_name);
            lv_obj_center(network_label);
            lv_obj_add_event_cb(network_button, network_button_event_callback, LV_EVENT_CLICKED, this);
        }

        static void network_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<WiFiSelectionScreen*>(lv_event_get_user_data(event));
            auto* button = static_cast<lv_obj_t*>(lv_event_get_target(event));
            auto* label = lv_obj_get_child(button, 0);
            screen->password_screen_.set_network_name(lv_label_get_text(label));
            ScreenManager::instance().set_screen(&screen->password_screen_);
        }
    };

}
