#pragma once

#include <cstdio>
#include <cstdlib>

#include "UIManager.hpp"
#include "../AppState.h"
#include "../network/ServerDiscovery.h"

namespace ui {

    class ServerScreen : public Screen {
    public:
        explicit ServerScreen(Screen* back_screen)
            : back_screen_(back_screen) {}

        ~ServerScreen() override {
            stop_scan_timer();
        }

        void build() override {
            root = lv_obj_create(nullptr);
            lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_pad_all(root, 0, 0);

            create_button("Back", 0, 0, 42, 18, back_button_event_callback);
            create_button("Scan", 43, 0, 42, 18, scan_button_event_callback);
            create_button("Manual", 86, 0, 42, 18, manual_button_event_callback);

            list_container_ = lv_obj_create(root);
            lv_obj_set_pos(list_container_, 0, 20);
            lv_obj_set_size(list_container_, 128, 44);
            lv_obj_set_scroll_dir(list_container_, LV_DIR_VER);
        }

        void on_show() override {
            show_discovered_servers();
            start_scan();
        }

        void on_hide() override {
            stop_scan_timer();
            discovery_.cancel();
            if (keyboard_ != nullptr) {
                lv_keyboard_set_textarea(keyboard_, nullptr);
                lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
            }
        }

    private:
        static constexpr uint32_t scan_timer_period_ms = 100;

        Screen* back_screen_ = nullptr;
        lv_obj_t* list_container_ = nullptr;
        lv_obj_t* keyboard_ = nullptr;
        lv_obj_t* address_textarea_ = nullptr;
        lv_obj_t* port_textarea_ = nullptr;
        lv_obj_t* connect_button_ = nullptr;
        lv_timer_t* scan_timer_ = nullptr;

        ServerDiscovery discovery_;

        using button_callback_t = void (*)(lv_event_t*);

        void create_button(
            const char* text,
            int32_t x,
            int32_t y,
            int32_t width,
            int32_t height,
            button_callback_t callback) {
            lv_obj_t* button = lv_button_create(root);
            apply_button_border(button);
            lv_obj_set_pos(button, x, y);
            lv_obj_set_size(button, width, height);
            lv_obj_t* label = lv_label_create(button);
            lv_label_set_text(label, text);
            lv_obj_center(label);
            lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, this);
        }

        void add_server_row(const char* text, uint32_t index) {
            lv_obj_t* button = lv_button_create(list_container_);
            apply_button_border(button);
            lv_obj_set_pos(button, 0, static_cast<int32_t>(index * 18));
            lv_obj_set_size(button, 128, 18);
            lv_obj_t* label = lv_label_create(button);
            lv_label_set_text(label, text);
            lv_obj_center(label);
            lv_obj_add_event_cb(button, server_button_event_callback, LV_EVENT_CLICKED, this);
        }

        void show_discovered_servers() {
            lv_obj_clean(list_container_);
            add_server_row("Scanning...", 0);
        }

        void start_scan() {
            stop_scan_timer();
            lv_obj_clean(list_container_);
            lv_obj_clear_flag(list_container_, LV_OBJ_FLAG_HIDDEN);
            discovery_.begin();
            add_server_row("Scanning...", 0);
            scan_timer_ = lv_timer_create(scan_timer_callback, scan_timer_period_ms, this);
        }

        void finish_scan() {
            const DiscoveredServerList& servers = discovery_.results();
            lv_obj_clean(list_container_);
            if (servers.empty()) {
                add_server_row("No servers found", 0);
                add_server_row("Use Manual", 1);
                return;
            }

            for (size_t index = 0; index < servers.size(); ++index) {
                char text[128];
                std::snprintf(
                    text,
                    sizeof(text),
                    "%s %s",
                    servers[index].name.c_str(),
                    servers[index].ip.toString().c_str());
                add_server_row(text, static_cast<uint32_t>(index));
            }
        }

        void poll_scan() {
            if (discovery_.update() != ServerDiscovery::Status::Scanning) {
                stop_scan_timer();
                finish_scan();
            }
        }

        void show_manual_fields() {
            lv_obj_clean(list_container_);
            lv_obj_add_flag(list_container_, LV_OBJ_FLAG_HIDDEN);

            address_textarea_ = lv_textarea_create(root);
            lv_obj_set_pos(address_textarea_, 0, 20);
            lv_obj_set_size(address_textarea_, 82, 18);
            lv_textarea_set_one_line(address_textarea_, true);
            lv_textarea_set_placeholder_text(address_textarea_, "IP address");

            port_textarea_ = lv_textarea_create(root);
            lv_obj_set_pos(port_textarea_, 84, 20);
            lv_obj_set_size(port_textarea_, 44, 18);
            lv_textarea_set_one_line(port_textarea_, true);
            lv_textarea_set_placeholder_text(port_textarea_, "Port");

            connect_button_ = lv_button_create(root);
            apply_button_border(connect_button_);
            lv_obj_set_pos(connect_button_, 0, 40);
            lv_obj_set_size(connect_button_, 128, 18);
            lv_obj_t* label = lv_label_create(connect_button_);
            lv_label_set_text(label, "Connect");
            lv_obj_center(label);
            lv_obj_add_event_cb(connect_button_, connect_button_event_callback, LV_EVENT_CLICKED, this);

            if (keyboard_ == nullptr) {
                keyboard_ = lv_keyboard_create(root);
                lv_obj_set_pos(keyboard_, 0, 18);
                lv_obj_set_size(keyboard_, 128, 46);
                lv_obj_set_style_pad_all(keyboard_, 0, 0);
                lv_obj_set_style_pad_gap(keyboard_, 0, 0);
                lv_obj_set_style_text_font(keyboard_, &lv_font_montserrat_8, LV_PART_ITEMS);
                apply_keyboard_button_border(keyboard_);
            }
            lv_obj_clear_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
            lv_keyboard_set_textarea(keyboard_, address_textarea_);
        }

        void connect_to_server(const char* host, uint16_t port) {
            if (AppState::get_instance().connect_server(host, port)) {
                ScreenManager::instance().set_screen(back_screen_);
            }
        }

        static void scan_timer_callback(lv_timer_t* timer) {
            auto* screen = static_cast<ServerScreen*>(lv_timer_get_user_data(timer));
            screen->poll_scan();
        }

        static void back_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
                auto* screen = static_cast<ServerScreen*>(lv_event_get_user_data(event));
                ScreenManager::instance().set_screen(screen->back_screen_);
            }
        }

        static void scan_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
                static_cast<ServerScreen*>(lv_event_get_user_data(event))->start_scan();
            }
        }

        static void manual_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
                static_cast<ServerScreen*>(lv_event_get_user_data(event))->show_manual_fields();
            }
        }

        static void server_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }
            auto* screen = static_cast<ServerScreen*>(lv_event_get_user_data(event));
            const uint32_t index = lv_obj_get_index(static_cast<lv_obj_t*>(lv_event_get_target(event)));
            const DiscoveredServer& server = screen->discovery_.results()[index];
            screen->connect_to_server(server.ip.toString().c_str(), server.port);
        }

        static void connect_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }
            auto* screen = static_cast<ServerScreen*>(lv_event_get_user_data(event));
            const char* port_text = lv_textarea_get_text(screen->port_textarea_);
            char* end = nullptr;
            const unsigned long port = strtoul(port_text, &end, 10);
            if (port_text[0] == '\0' || end == port_text || *end != '\0' || port == 0 || port > 65535) {
                return;
            }
            screen->connect_to_server(
                lv_textarea_get_text(screen->address_textarea_), static_cast<uint16_t>(port));
        }

        void stop_scan_timer() {
            if (scan_timer_ != nullptr) {
                lv_timer_del(scan_timer_);
                scan_timer_ = nullptr;
            }
        }
    };

}
