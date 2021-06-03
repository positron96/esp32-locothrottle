#include "Encoder.h"

void IRAM_ATTR Encoder::isr_handler_static(void* arg) {
    reinterpret_cast<Encoder*>(arg)->tick();
}


void Encoder::start() {    
    esp_timer_create_args_t _timerConfig;
    _timerConfig.arg = reinterpret_cast<void*>(this);
    _timerConfig.callback = isr_handler_static;
    _timerConfig.dispatch_method = ESP_TIMER_TASK;
    _timerConfig.name = "Encoder";
    if (_timer) {
        esp_timer_stop(_timer);
        esp_timer_delete(_timer);
    }
    esp_timer_create(&_timerConfig, &_timer);  
    esp_timer_start_periodic(_timer, 500ULL);
}

void Encoder::stop() {
    if (_timer) {
        esp_timer_stop(_timer);
        esp_timer_delete(_timer);
        _timer = nullptr;
    }
}

void IRAM_ATTR Encoder::tick() {

}

