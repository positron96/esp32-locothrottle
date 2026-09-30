#include "Window.h"

void Window::loadScreen() {
    if (screen_ == nullptr) {
        screen_ = lv_obj_create(nullptr);
    }
    lv_screen_load(screen_);
}
