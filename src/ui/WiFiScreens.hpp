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

            back_button_ = lv_button_create(root);
            apply_button_border(back_button_);
            lv_obj_set_pos(back_button_, 0, 0);
            lv_obj_set_size(back_button_, 42, 18);
            lv_obj_t* back_label = lv_label_create(back_button_);
            lv_label_set_text(back_label, "Back");
            lv_obj_center(back_label);
            lv_obj_add_event_cb(back_button_, back_button_event_callback, LV_EVENT_CLICKED, this);

            ok_button_ = lv_button_create(root);
            apply_button_border(ok_button_);
            lv_obj_set_pos(ok_button_, 86, 0);
            lv_obj_set_size(ok_button_, 42, 18);
            lv_obj_t* ok_label = lv_label_create(ok_button_);
            lv_label_set_text(ok_label, "OK");
            lv_obj_center(ok_label);
            lv_obj_add_event_cb(ok_button_, ok_button_event_callback, LV_EVENT_CLICKED, this);

            password_textarea_ = lv_textarea_create(root);
            lv_obj_set_pos(password_textarea_, 44, 0);
            lv_obj_set_size(password_textarea_, 42, 18);
            lv_textarea_set_password_mode(password_textarea_, true);
            lv_textarea_set_one_line(password_textarea_, true);

            keyboard_ = lv_keyboard_create(root);
            lv_obj_set_align(keyboard_, LV_ALIGN_TOP_LEFT);
            lv_obj_set_pos(keyboard_, 0, 18);
            lv_obj_set_size(keyboard_, 128, 46);
            lv_obj_set_style_pad_all(keyboard_, 0, 0);
            lv_obj_set_style_pad_gap(keyboard_, 0, 0);
            lv_obj_set_style_pad_all(keyboard_, 0, LV_PART_ITEMS);
            lv_obj_set_style_text_font(keyboard_, &u8g2_font_nokiafc22_tf, LV_PART_ITEMS);
            apply_keyboard_button_border(keyboard_);
            lv_keyboard_set_textarea(keyboard_, password_textarea_);

            create_keyboard_action_button("BS", 110, 18, backspace_button_event_callback);
            create_keyboard_action_button("<", 18, 52, cursor_left_button_event_callback);
            create_keyboard_action_button(">", 90, 52, cursor_right_button_event_callback);
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
        lv_obj_t* back_button_ = nullptr;
        lv_obj_t* ok_button_ = nullptr;
        lv_obj_t* password_textarea_ = nullptr;
        lv_obj_t* keyboard_ = nullptr;
        String network_name_;

        using keyboard_action_callback_t = void (*)(lv_event_t*);

        void create_keyboard_action_button(
            const char* text,
            int32_t x,
            int32_t y,
            keyboard_action_callback_t callback) {
            lv_obj_t* button = lv_btn_create(root);
            apply_button_border(button);
            lv_obj_set_pos(button, x, y);
            lv_obj_set_size(button, 18, 12);
            lv_obj_set_style_pad_all(button, 0, 0);
            lv_obj_set_style_text_font(button, &u8g2_font_nokiafc22_tf, 0);
            lv_obj_t* label = lv_label_create(button);
            lv_label_set_text(label, text);
            lv_obj_center(label);
            lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, this);
        }

        static void backspace_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<WiFiPasswordScreen*>(lv_event_get_user_data(event));
            lv_textarea_delete_char(screen->password_textarea_);
        }

        static void cursor_left_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<WiFiPasswordScreen*>(lv_event_get_user_data(event));
            lv_textarea_cursor_left(screen->password_textarea_);
        }

        static void cursor_right_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<WiFiPasswordScreen*>(lv_event_get_user_data(event));
            lv_textarea_cursor_right(screen->password_textarea_);
        }

        static void back_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<WiFiPasswordScreen*>(lv_event_get_user_data(event));
            ScreenManager::instance().set_screen(screen->back_screen_);
        }

        static void ok_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<WiFiPasswordScreen*>(lv_event_get_user_data(event));
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

            back_button_ = lv_button_create(root);
            apply_button_border(back_button_);
            lv_obj_set_pos(back_button_, 0, 0);
            lv_obj_set_size(back_button_, 42, 18);
            lv_obj_t* back_label = lv_label_create(back_button_);
            lv_label_set_text(back_label, "Back");
            lv_obj_center(back_label);
            lv_obj_add_event_cb(back_button_, back_button_event_callback, LV_EVENT_CLICKED, this);

            network_container_ = lv_obj_create(root);
            lv_obj_set_pos(network_container_, 0, 20);
            lv_obj_set_size(network_container_, 128, 44);
            lv_obj_set_scroll_dir(network_container_, LV_DIR_VER);
        }

        void on_show() override {
            start_scan();
        }

        void on_hide() override {
            stop_scan_timer();
            WiFi.scanDelete();
        }

    private:
        Screen* back_screen_ = nullptr;
        WiFiPasswordScreen password_screen_;
        lv_obj_t* back_button_ = nullptr;
        lv_obj_t* network_container_ = nullptr;
        lv_timer_t* scan_timer_ = nullptr;
        uint32_t network_row_count_ = 0;

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
            network_row_count_ = 0;
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
            lv_obj_set_pos(network_button, 0, static_cast<int32_t>(network_row_count_ * 18));
            lv_obj_set_size(network_button, 128, 18);
            lv_obj_t* network_label = lv_label_create(network_button);
            lv_label_set_text(network_label, network_name);
            lv_obj_center(network_label);
            lv_obj_add_event_cb(network_button, network_button_event_callback, LV_EVENT_CLICKED, this);
            ++network_row_count_;
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
