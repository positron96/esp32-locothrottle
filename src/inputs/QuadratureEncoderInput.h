#pragma once

#include <driver/pulse_cnt.h>
#include <etl/debounce.h>
#include <lvgl.h>

class QuadratureEncoderInput {
public:
    bool begin();

    lv_indev_t* device() const;

    void register_lvgl_indev();

private:
    static void readCallback(lv_indev_t* indev, lv_indev_data_t* data);

    pcnt_unit_handle_t unit_ = nullptr;
    lv_indev_t* device_ = nullptr;
    etl::debounce<2> buttonDebounce_;
};
