#pragma once

#include "UIManager.hpp"
#include "../AppState.h"
#include "ServerScreens.hpp"
#include "WiFiScreens.hpp"
#include <WiFi.h>

namespace ui {

    class StatusScreen : public Screen {
    public:
        StatusScreen()
            : wifi_selection_screen_(this), server_screen_(this) {}

        ~StatusScreen() override {
            stop_refresh_timer();
        }

        void build() override {
            root = lv_obj_create(nullptr);
            lv_obj_set_layout(root, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                root,
                LV_FLEX_ALIGN_SPACE_BETWEEN,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START);
            lv_obj_set_style_pad_all(root, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_bottom(root, 2, LV_PART_MAIN);

            lv_obj_t* status_group = lv_obj_create(root);
            lv_obj_set_size(status_group, 128, 24);
            lv_obj_set_style_border_width(status_group, 0, LV_PART_MAIN);
            lv_obj_set_layout(status_group, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(status_group, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                status_group,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START);
            lv_obj_set_style_pad_row(status_group, 8, LV_PART_MAIN);

            wifi_label_ = create_label("WiFi: Not set", status_group);
            lv_obj_add_flag(wifi_label_, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(wifi_label_, wifi_label_event_callback, LV_EVENT_CLICKED, this);
            server_label_ = create_label("TCP: Not set", status_group);
            lv_obj_add_flag(server_label_, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(server_label_, server_label_event_callback, LV_EVENT_CLICKED, this);

            control_button_ = lv_btn_create(root);
            apply_button_border(control_button_);
            lv_obj_set_size(control_button_, 128, 28);
            control_label_ = lv_label_create(control_button_);
            lv_label_set_text(control_label_, "To control");
            lv_obj_center(control_label_);

            refresh_status();
            refresh_timer_ = lv_timer_create(refresh_timer_callback, status_timer_period_ms, this);

        };

        void on_show() override {
            refresh_status();
            if (refresh_timer_ == nullptr) {
                refresh_timer_ = lv_timer_create(refresh_timer_callback, status_timer_period_ms, this);
            }
        }

        void on_hide() override {
            stop_refresh_timer();
        }

    private:
        static constexpr uint32_t status_timer_period_ms = 200;

        lv_obj_t* wifi_label_ = nullptr;
        lv_obj_t* server_label_ = nullptr;
        lv_obj_t* control_button_ = nullptr;
        lv_obj_t* control_label_ = nullptr;
        lv_timer_t* refresh_timer_ = nullptr;
        WiFiSelectionScreen wifi_selection_screen_;
        ServerScreen server_screen_;

        static void wifi_label_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<StatusScreen*>(lv_event_get_user_data(event));
            ScreenManager::instance().set_screen(&screen->wifi_selection_screen_);
        }

        static void server_label_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }

            auto* screen = static_cast<StatusScreen*>(lv_event_get_user_data(event));
            ScreenManager::instance().set_screen(&screen->server_screen_);
        }

        lv_obj_t* create_label(const char* text, lv_obj_t* parent) {
            lv_obj_t* label = lv_label_create(parent);
            lv_label_set_text(label, text);
            return label;
        }

        static void refresh_timer_callback(lv_timer_t* timer) {
            auto* screen = static_cast<StatusScreen*>(lv_timer_get_user_data(timer));
            screen->refresh_status();
        }

        void refresh_status() {
            const bool wifi_connected = is_wifi_connected();
            const bool server_connected = AppState::get_instance().is_server_connected();

            lv_label_set_text(wifi_label_, wifi_status_text());
            lv_label_set_text(server_label_, server_connected ? "TCP: Connected" : "TCP: Not set");

            if (wifi_connected && server_connected) {
                //lv_obj_clear_state(control_button_, LV_STATE_DISABLED);
            } else {
                //lv_obj_add_state(control_button_, LV_STATE_DISABLED);
            }
        }

        bool is_wifi_connected() const {
            return WiFi.status() == WL_CONNECTED;
        }

        const char* wifi_status_text() const {
            if (WiFi.status() == WL_CONNECTED) {
                return "WiFi: Connected";
            }
            if (WiFi.SSID().isEmpty()) {
                return "WiFi: Not set";
            }
            return "WiFi: Connecting";
        }

        void stop_refresh_timer() {
            if (refresh_timer_ != nullptr) {
                lv_timer_del(refresh_timer_);
                refresh_timer_ = nullptr;
            }
        }

    };
}
