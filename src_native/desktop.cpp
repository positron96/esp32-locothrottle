#include <windows.h>

#include <lvgl.h>
#include <src/drivers/windows/lv_windows_input.h>

#include "ui/StatusScreen.hpp"

namespace {

constexpr int32_t DISPLAY_HOR_RES = WTH_DISPLAY_HOR_RES;
constexpr int32_t DISPLAY_VER_RES = WTH_DISPLAY_VER_RES;

void initDisplay() {
    lv_init();
    lv_display_t* display = lv_windows_create_display(
        L"WiThRemote",
        DISPLAY_HOR_RES,
        DISPLAY_VER_RES,
        500,
        false,
        true);
    lv_windows_acquire_pointer_indev(display);
    lv_windows_acquire_encoder_indev(display);
}

} // namespace

int main() {
    initDisplay();
    ui::ScreenManager::instance().init();

    ui::StatusScreen status_screen;
    ui::ScreenManager::instance().set_screen(&status_screen);

    while(true) {
        lv_timer_handler();
        Sleep(5);
    }
}
