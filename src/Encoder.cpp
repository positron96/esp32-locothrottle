#include "Encoder.h"

void IRAM_ATTR Encoder::isr_handler_static(void* arg) {
    reinterpret_cast<Encoder*>(arg)->tick();
}


void Encoder::start() {    

    pinMode(pin1, INPUT_PULLUP);
    pinMode(pin2, INPUT_PULLUP);

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
    esp_timer_start_periodic(_timer, TIMER_PERIOD);
}

void Encoder::stop() {
    if (_timer) {
        esp_timer_stop(_timer);
        esp_timer_delete(_timer);
        _timer = nullptr;
    }
}

static const int8_t QEM [16] = {0,-1,1,0,  1,0,0,-1,  -1,0,0,1,  0,1,-1,0}; 

void IRAM_ATTR Encoder::tick() {
    uint8_t st = digitalRead(this->pin1) << 1 | digitalRead(pin2);
    if(st != newState) {
        //Serial.printf("\nstate = %d%d\n", st>>1, st&1);
        newState = st;
        stateCount = TICKS_REQ;        
    }
    if(st == newState && newState!=stableState) {
        if(stateCount>0) {
            stateCount --;
            //Serial.printf(".");
        } else {
            pos += QEM[st<<2 | stableState];
            stableState = newState;
            //Serial.printf("pos = %d\n", pos);
        }
    }
}

