#include <windows.h>

#include <lvgl.h>

#include "ui/StatusScreen.hpp"

namespace {

constexpr int32_t DISPLAY_HOR_RES = WTH_DISPLAY_HOR_RES;
constexpr int32_t DISPLAY_VER_RES = WTH_DISPLAY_VER_RES;

void initDisplay() {
    lv_init();
    lv_windows_create_display(
        L"WiThRemote",
        DISPLAY_HOR_RES,
        DISPLAY_VER_RES,
        500,
        false,
        true);

    lv_theme_t * mono = lv_theme_mono_init(lv_disp_get_default(), /*dark_bg=*/true, LV_FONT_DEFAULT);
    lv_disp_set_theme(lv_disp_get_default(), mono);

}

} // namespace

int main() {
    initDisplay();

    ui::StatusScreen status_screen;
    ui::ScreenManager::instance().set_screen(&status_screen);

    while(true) {
        lv_timer_handler();
        Sleep(5);
    }
}
