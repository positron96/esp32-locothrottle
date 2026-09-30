#include "ui/demo_ui.h"

#include <lvgl.h>

#include "display/mono_display.h"

namespace demo_ui {
namespace {

constexpr uint32_t SCREEN_DWELL_MS = 4000;
constexpr uint32_t UPDATE_PERIOD_MS = 100;

constexpr const char * MENU_ITEMS[] = {"Select loco", "Turnouts", "Network", "Settings"};
constexpr size_t MENU_ITEM_CNT = sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);

struct Widgets {
    lv_obj_t * splash_bar;
    lv_obj_t * splash_hint;
    lv_obj_t * speed_value;
    lv_obj_t * speed_bar;
    lv_obj_t * direction;
    lv_obj_t * menu_rows[MENU_ITEM_CNT];
};

Widgets widgets{};
lv_obj_t * screens[3] = {nullptr, nullptr, nullptr};
size_t active_screen = 0;

uint32_t progress = 0;
int32_t speed = 0;
int32_t speed_step = 4;
size_t menu_selection = 0;

/** Screen root: no padding, no scrolling - every pixel counts on 128x64. */
lv_obj_t * make_screen() {
    lv_obj_t * scr = lv_obj_create(nullptr);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_radius(scr, 0, 0);
    return scr;
}

/** Inverted (white on black background is the default, so: black on white) title bar. */
lv_obj_t * make_title(lv_obj_t * parent, const char * text) {
    lv_obj_t * bar = lv_obj_create(parent);
    lv_obj_remove_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(bar, lv_pct(100), 13);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(bar, lv_color_white(), 0);

    lv_obj_t * label = lv_label_create(bar);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_center(label);
    return bar;
}

lv_obj_t * build_splash_screen() {
    lv_obj_t * scr = make_screen();

    // unscii_16 is 16 px wide per glyph, so only short strings fit across 128 px.
    lv_obj_t * title = lv_label_create(scr);
    lv_label_set_text(title, "WiThRemote");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t * subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "LVGL mono demo");
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 26);

    widgets.splash_bar = lv_bar_create(scr);
    lv_obj_set_size(widgets.splash_bar, 108, 8);
    lv_obj_align(widgets.splash_bar, LV_ALIGN_TOP_MID, 0, 42);
    lv_bar_set_range(widgets.splash_bar, 0, 100);

    widgets.splash_hint = lv_label_create(scr);
    lv_label_set_text(widgets.splash_hint, "connecting...");
    lv_obj_align(widgets.splash_hint, LV_ALIGN_BOTTOM_MID, 0, -2);

    return scr;
}

lv_obj_t * build_throttle_screen() {
    lv_obj_t * scr = make_screen();
    make_title(scr, "LOCO 1234");

    widgets.speed_value = lv_label_create(scr);
    lv_obj_set_style_text_font(widgets.speed_value, &lv_font_unscii_16, 0);
    lv_label_set_text(widgets.speed_value, "0");
    lv_obj_align(widgets.speed_value, LV_ALIGN_LEFT_MID, 8, 0);

    lv_obj_t * unit = lv_label_create(scr);
    lv_label_set_text(unit, "%");
    lv_obj_align_to(unit, widgets.speed_value, LV_ALIGN_OUT_RIGHT_BOTTOM, 2, 0);

    widgets.direction = lv_label_create(scr);
    lv_label_set_text(widgets.direction, "FWD >>");
    lv_obj_align(widgets.direction, LV_ALIGN_RIGHT_MID, -6, 0);

    widgets.speed_bar = lv_bar_create(scr);
    lv_obj_set_size(widgets.speed_bar, 116, 10);
    lv_obj_align(widgets.speed_bar, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_bar_set_range(widgets.speed_bar, 0, 100);

    return scr;
}

lv_obj_t * build_menu_screen() {
    lv_obj_t * scr = make_screen();
    lv_obj_t * title = make_title(scr, "MENU");

    lv_obj_t * list = lv_obj_create(scr);
    lv_obj_remove_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(list, lv_pct(100), mono_display::HEIGHT - 13);
    lv_obj_align_to(list, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_radius(list, 0, 0);
    lv_obj_set_style_pad_all(list, 1, 0);
    lv_obj_set_style_pad_row(list, 0, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    for(size_t i = 0; i < MENU_ITEM_CNT; ++i) {
        lv_obj_t * row = lv_label_create(list);
        lv_label_set_text(row, MENU_ITEMS[i]);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_style_pad_ver(row, 1, 0);
        lv_obj_set_style_pad_hor(row, 2, 0);
        lv_obj_set_style_bg_color(row, lv_color_white(), 0);
        lv_obj_set_style_text_color(row, lv_color_black(), 0);
        widgets.menu_rows[i] = row;
    }
    return scr;
}

void highlight_menu_row(size_t index) {
    for(size_t i = 0; i < MENU_ITEM_CNT; ++i) {
        const bool selected = (i == index);
        lv_obj_set_style_bg_opa(widgets.menu_rows[i], selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_text_color(widgets.menu_rows[i],
                                    selected ? lv_color_black() : lv_color_white(), 0);
    }
}

void update_cb(lv_timer_t *) {
    progress = (progress + 2) % 101;
    lv_bar_set_value(widgets.splash_bar, static_cast<int32_t>(progress), LV_ANIM_OFF);
    lv_label_set_text(widgets.splash_hint, progress < 50 ? "connecting..." : "scanning...");

    speed += speed_step;
    if(speed >= 100 || speed <= 0) {
        speed = speed < 0 ? 0 : (speed > 100 ? 100 : speed);
        speed_step = -speed_step;
        lv_label_set_text(widgets.direction, speed_step > 0 ? "FWD >>" : "<< REV");
    }
    lv_label_set_text_fmt(widgets.speed_value, "%d", static_cast<int>(speed));
    lv_bar_set_value(widgets.speed_bar, speed, LV_ANIM_OFF);
}

void cycle_cb(lv_timer_t *) {
    active_screen = (active_screen + 1) % (sizeof(screens) / sizeof(screens[0]));
    if(screens[active_screen] == screens[2]) {
        menu_selection = (menu_selection + 1) % MENU_ITEM_CNT;
        highlight_menu_row(menu_selection);
    }
    // No animation: the panel has no gray levels to make a transition look good.
    lv_screen_load_anim(screens[active_screen], LV_SCREEN_LOAD_ANIM_OVER_LEFT, 500, 0, false);
}

}  // namespace

void start() {
    screens[0] = build_splash_screen();
    screens[1] = build_throttle_screen();
    screens[2] = build_menu_screen();
    highlight_menu_row(menu_selection);

    lv_timer_create(update_cb, UPDATE_PERIOD_MS, nullptr);
    lv_timer_create(cycle_cb, SCREEN_DWELL_MS, nullptr);

    lv_screen_load(screens[0]);
}

}  // namespace demo_ui
