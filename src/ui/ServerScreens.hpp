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
            lv_obj_set_layout(root, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                root,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START);

            header_container_ = lv_obj_create(root);
            lv_obj_set_size(header_container_, 128, 12);
            lv_obj_set_style_border_width(header_container_, 0, LV_PART_MAIN);
            lv_obj_set_layout(header_container_, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(header_container_, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(
                header_container_,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START);
            lv_obj_set_style_pad_column(header_container_, 1, LV_PART_MAIN);

            back_button_ = create_button("Back", 42, back_button_event_callback);
            scan_button_ = create_button("Scan", 42, scan_button_event_callback);
            manual_button_ = create_button("Manual", 42, manual_button_event_callback);

            list_container_ = lv_obj_create(root);
            lv_obj_set_width(list_container_, 128);
            lv_obj_set_flex_grow(list_container_, 1);
            lv_obj_set_scroll_dir(list_container_, LV_DIR_VER);
            lv_obj_set_layout(list_container_, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(list_container_, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                list_container_,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START);

            manual_fields_container_ = lv_obj_create(root);
            lv_obj_set_size(manual_fields_container_, 128, 18);
            lv_obj_add_flag(manual_fields_container_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_border_width(manual_fields_container_, 0, LV_PART_MAIN);
            lv_obj_set_layout(manual_fields_container_, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(manual_fields_container_, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(
                manual_fields_container_,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START,
                LV_FLEX_ALIGN_START);
            lv_obj_set_style_pad_column(manual_fields_container_, 2, LV_PART_MAIN);

            keyboard_ = lv_keyboard_create(root);
            lv_obj_set_size(keyboard_, 128, 46);
            lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_IGNORE_LAYOUT);
            lv_obj_align(keyboard_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
            lv_obj_set_style_pad_all(keyboard_, 0, 0);
            lv_obj_set_style_pad_gap(keyboard_, 0, 0);
            lv_obj_set_style_pad_all(keyboard_, 0, LV_PART_ITEMS);
            lv_obj_add_event_cb(keyboard_, manual_keyboard_event_callback, LV_EVENT_READY, this);
            lv_obj_add_event_cb(keyboard_, manual_keyboard_event_callback, LV_EVENT_CANCEL, this);
        }

        void on_show() override {
            show_discovered_servers();
            start_scan();
        }

        void on_hide() override {
            stop_scan_timer();
            discovery_.cancel();
            if (keyboard_ != nullptr) {
                if (keyboard_in_group_) {
                    lv_group_remove_obj(keyboard_);
                    keyboard_in_group_ = false;
                }
                lv_keyboard_set_textarea(keyboard_, nullptr);
                lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
            }
            lv_obj_add_flag(manual_fields_container_, LV_OBJ_FLAG_HIDDEN);
            clear_manual_fields();
            lv_obj_clear_flag(list_container_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(header_container_, LV_OBJ_FLAG_HIDDEN);
        }

    private:
        static constexpr uint32_t scan_timer_period_ms = 100;
        static constexpr int32_t list_row_height = 10;

        Screen* back_screen_ = nullptr;
        lv_obj_t* back_button_ = nullptr;
        lv_obj_t* scan_button_ = nullptr;
        lv_obj_t* manual_button_ = nullptr;
        lv_obj_t* header_container_ = nullptr;
        lv_obj_t* list_container_ = nullptr;
        lv_obj_t* manual_fields_container_ = nullptr;
        lv_obj_t* keyboard_ = nullptr;
        lv_obj_t* address_textarea_ = nullptr;
        lv_obj_t* port_textarea_ = nullptr;
        lv_timer_t* scan_timer_ = nullptr;
        char saved_server_host_[64] = {};
        uint16_t saved_server_port_ = 0;
        bool saved_server_available_ = false;
        bool keyboard_in_group_ = false;

        ServerDiscovery discovery_;

        using button_callback_t = void (*)(lv_event_t*);

        lv_obj_t* create_button(
            const char* text,
            int32_t width,
            button_callback_t callback) {
            lv_obj_t* button = lv_button_create(header_container_);
            apply_button_border(button);
            lv_obj_set_width(button, width);
            lv_obj_t* label = lv_label_create(button);
            lv_label_set_text(label, text);
            lv_obj_center(label);
            add_focusable(button);
            lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, this);
            return button;
        }

        void add_server_row(const char* text, button_callback_t callback = nullptr) {
            lv_obj_t* button = lv_button_create(list_container_);
            apply_button_border(button);
            lv_obj_set_size(button, 128, list_row_height);
            lv_obj_t* label = lv_label_create(button);
            lv_label_set_text(label, text);
            lv_obj_center(label);
            if (callback != nullptr) {
                add_focusable(button);
                lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, this);
            }
        }

        void show_discovered_servers() {
            lv_obj_clean(list_container_);
            add_server_row("Scanning...");
        }

        void start_scan() {
            stop_scan_timer();
            lv_obj_clean(list_container_);
            lv_obj_clear_flag(list_container_, LV_OBJ_FLAG_HIDDEN);
            discovery_.begin();
            add_server_row("Scanning...");
            scan_timer_ = lv_timer_create(scan_timer_callback, scan_timer_period_ms, this);
        }

        void finish_scan() {
            const DiscoveredServerList& servers = discovery_.results();
            lv_obj_clean(list_container_);
            saved_server_available_ = AppState::get_instance().get_saved_server(
                saved_server_host_, sizeof(saved_server_host_), saved_server_port_);
            if (saved_server_available_) {
                add_server_row("Saved server", server_button_event_callback);
            }

            if (servers.empty()) {
                add_server_row("No servers found");
                add_server_row("Use Manual", manual_button_event_callback);
                return;
            }

            for (size_t index = 0; index < servers.size(); ++index) {
                char text[128];
                std::snprintf(
                    text,
                    sizeof(text),
                    "%s %s:%u",
                    servers[index].name.c_str(),
                    servers[index].ip.toString().c_str(),
                    static_cast<unsigned>(servers[index].port));
                add_server_row(
                    text,
                    server_button_event_callback);
            }
        }

        void poll_scan() {
            if (discovery_.update() != ServerDiscovery::Status::Scanning) {
                stop_scan_timer();
                finish_scan();
            }
        }

        void show_manual_fields() {
            stop_scan_timer();
            discovery_.cancel();
            lv_obj_add_flag(list_container_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(header_container_, LV_OBJ_FLAG_HIDDEN);

            address_textarea_ = lv_textarea_create(manual_fields_container_);
            add_focusable(address_textarea_);
            lv_obj_set_size(address_textarea_, 82, 18);
            lv_textarea_set_one_line(address_textarea_, true);
            lv_textarea_set_placeholder_text(address_textarea_, "IP address");
            lv_obj_add_event_cb(address_textarea_, manual_textarea_event_callback, LV_EVENT_CLICKED, this);

            port_textarea_ = lv_textarea_create(manual_fields_container_);
            add_focusable(port_textarea_);
            lv_obj_set_size(port_textarea_, 44, 18);
            lv_textarea_set_one_line(port_textarea_, true);
            lv_textarea_set_placeholder_text(port_textarea_, "Port");
            lv_obj_add_event_cb(port_textarea_, manual_textarea_event_callback, LV_EVENT_CLICKED, this);

            lv_obj_clear_flag(manual_fields_container_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
            add_focusable(keyboard_, false);
            keyboard_in_group_ = true;
            lv_keyboard_set_textarea(keyboard_, address_textarea_);
            lv_group_focus_obj(address_textarea_);
        }

        void clear_manual_fields() {
            if (address_textarea_ != nullptr) {
                lv_obj_delete(address_textarea_);
                address_textarea_ = nullptr;
            }
            if (port_textarea_ != nullptr) {
                lv_obj_delete(port_textarea_);
                port_textarea_ = nullptr;
            }
        }

        void return_to_server_list() {
            if (keyboard_in_group_) {
                lv_group_remove_obj(keyboard_);
                keyboard_in_group_ = false;
            }
            lv_keyboard_set_textarea(keyboard_, nullptr);
            lv_obj_add_flag(keyboard_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(manual_fields_container_, LV_OBJ_FLAG_HIDDEN);
            clear_manual_fields();
            lv_obj_clear_flag(header_container_, LV_OBJ_FLAG_HIDDEN);
            start_scan();
        }

        void connect_manual_server() {
            const char* port_text = lv_textarea_get_text(port_textarea_);
            char* end = nullptr;
            const unsigned long port = strtoul(port_text, &end, 10);
            if (port_text[0] == '\0' || end == port_text || *end != '\0' || port == 0 || port > 65535) {
                return;
            }
            connect_to_server(
                lv_textarea_get_text(address_textarea_), static_cast<uint16_t>(port));
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

        static void manual_textarea_event_callback(lv_event_t* event) {
            auto* screen = static_cast<ServerScreen*>(lv_event_get_user_data(event));
            lv_keyboard_set_textarea(
                screen->keyboard_, static_cast<lv_obj_t*>(lv_event_get_target(event)));
        }

        static void manual_keyboard_event_callback(lv_event_t* event) {
            auto* screen = static_cast<ServerScreen*>(lv_event_get_user_data(event));
            if (lv_event_get_code(event) == LV_EVENT_CANCEL) {
                screen->return_to_server_list();
            }
            else if (lv_event_get_code(event) == LV_EVENT_READY) {
                screen->connect_manual_server();
            }
        }

        static void server_button_event_callback(lv_event_t* event) {
            if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
                return;
            }
            auto* screen = static_cast<ServerScreen*>(lv_event_get_user_data(event));
            uint32_t index = lv_obj_get_index(static_cast<lv_obj_t*>(lv_event_get_target(event)));
            if (screen->saved_server_available_) {
                if (index == 0) {
                    screen->connect_to_server(screen->saved_server_host_, screen->saved_server_port_);
                    return;
                }
                --index;
            }
            if (index >= screen->discovery_.results().size()) {
                return;
            }
            const DiscoveredServer& server = screen->discovery_.results()[index];
            screen->connect_to_server(server.ip.toString().c_str(), server.port);
        }

        void stop_scan_timer() {
            if (scan_timer_ != nullptr) {
                lv_timer_del(scan_timer_);
                scan_timer_ = nullptr;
            }
        }
    };

}
