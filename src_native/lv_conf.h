#pragma once

#include "../include/lv_conf.h"

// Native Windows preview target: 128x64 logical pixels.
#define WTH_DISPLAY_HOR_RES 128
#define WTH_DISPLAY_VER_RES 64

// The Win32 backend requires a packed 16- or 32-bit framebuffer.
#undef LV_COLOR_DEPTH
#define LV_COLOR_DEPTH 16
#undef LV_COLOR_FORMAT_DEFAULT
#define LV_COLOR_FORMAT_DEFAULT LV_COLOR_FORMAT_RGB565

#undef LV_DRAW_SW_SUPPORT_RGB565
#define LV_DRAW_SW_SUPPORT_RGB565 1

#undef LV_USE_OS
#define LV_USE_OS LV_OS_WINDOWS
#undef LV_USE_WINDOWS
#define LV_USE_WINDOWS 1

// Keep the native preview independent of desktop graphics stacks.
#undef LV_USE_SDL
#define LV_USE_SDL 0
#undef LV_USE_GLFW
#define LV_USE_GLFW 0
#undef LV_USE_OPENGLES
#define LV_USE_OPENGLES 0
#undef LV_USE_WAYLAND
#define LV_USE_WAYLAND 0
#undef LV_USE_X11
#define LV_USE_X11 0

#undef LV_USE_THEME_DEFAULT
#define LV_USE_THEME_DEFAULT 0
#undef LV_USE_THEME_MONO
#define LV_USE_THEME_MONO 1
#undef LV_USE_BUTTON
#define LV_USE_BUTTON 1
#undef LV_USE_BUTTONMATRIX
#define LV_USE_BUTTONMATRIX 1
#undef LV_USE_LABEL
#define LV_USE_LABEL 1
#undef LV_USE_KEYBOARD
#define LV_USE_KEYBOARD 1
#undef LV_USE_TEXTAREA
#define LV_USE_TEXTAREA 1
