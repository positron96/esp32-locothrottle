#include "QuadratureEncoderInput.h"

#include <Arduino.h>

#include "../pins.h"

namespace {

constexpr uint32_t ENCODER_GLITCH_FILTER_NS = 12000;

} // namespace

bool QuadratureEncoderInput::begin() {
    pinMode(ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_B_PIN, INPUT_PULLUP);
    pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);

    const pcnt_unit_config_t unitConfig = {
        .low_limit = -32768,
        .high_limit = 32767,
        .intr_priority = 0,
        .flags = {.accum_count = 0},
    };
    if (pcnt_new_unit(&unitConfig, &unit_) != ESP_OK) {
        Serial.println(F("[Encoder] Failed to create pulse counter unit"));
        return false;
    }

    const pcnt_glitch_filter_config_t filterConfig = {
        .max_glitch_ns = ENCODER_GLITCH_FILTER_NS,
    };
    if (pcnt_unit_set_glitch_filter(unit_, &filterConfig) != ESP_OK) {
        Serial.println(F("[Encoder] Failed to configure pulse filter"));
        return false;
    }

    const pcnt_chan_config_t channelConfig = {
        .edge_gpio_num = ENCODER_A_PIN,
        .level_gpio_num = ENCODER_B_PIN,
        .flags = {},
    };
    pcnt_channel_handle_t channel = nullptr;
    if (pcnt_new_channel(unit_, &channelConfig, &channel) != ESP_OK ||
        pcnt_channel_set_edge_action(channel, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                     PCNT_CHANNEL_EDGE_ACTION_DECREASE) != ESP_OK ||
        pcnt_channel_set_level_action(channel, PCNT_CHANNEL_LEVEL_ACTION_INVERSE,
                                      PCNT_CHANNEL_LEVEL_ACTION_KEEP) != ESP_OK ||
        pcnt_unit_enable(unit_) != ESP_OK || pcnt_unit_clear_count(unit_) != ESP_OK || pcnt_unit_start(unit_) != ESP_OK) {
        Serial.println(F("[Encoder] Failed to initialize pulse counter"));
        return false;
    }

    Serial.printf("[Encoder] Ready on GPIO %u/%u, button GPIO %u\n", ENCODER_A_PIN, ENCODER_B_PIN,
                  ENCODER_BUTTON_PIN);
    return true;
}

void QuadratureEncoderInput::register_lvgl_indev() {
    if (device_ == nullptr) {
        device_ = lv_indev_create();
        lv_indev_set_type(device_, LV_INDEV_TYPE_ENCODER);
        lv_indev_set_read_cb(device_, readCallback);
        lv_indev_set_user_data(device_, this);
    }
}

lv_indev_t* QuadratureEncoderInput::device() const {
    return device_;
}

void QuadratureEncoderInput::readCallback(lv_indev_t* indev, lv_indev_data_t* data) {
    auto* self = static_cast<QuadratureEncoderInput*>(lv_indev_get_user_data(indev));
    data->enc_diff = 0;
    if (self == nullptr) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    int count = 0;
    if (pcnt_unit_get_count(self->unit_, &count) == ESP_OK) {
        data->enc_diff = static_cast<int32_t>(count);
        pcnt_unit_clear_count(self->unit_);
    }

    self->buttonDebounce_.add(digitalRead(ENCODER_BUTTON_PIN) == LOW);
    data->state = self->buttonDebounce_.is_set() ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
