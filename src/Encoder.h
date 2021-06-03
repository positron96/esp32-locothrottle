#pragma once

#include <cstdint>
#include <Arduino.h>

extern "C" {
  #include "esp_timer.h"
}

class Encoder {
public:
    Encoder(uint8_t pin1, uint8_t pin2): 
        pin1(pin1), pin2(pin2), pos(0)
    {
    }

    ~Encoder() { stop(); }

    void start() ;

    void stop();

    int32_t getCount() { return pos/2; }

private:
    uint8_t pin1, pin2;
    int32_t pos;
    uint32_t stateCount;
    uint8_t stableState, newState;

    static constexpr uint32_t TIMER_PERIOD = 500ul;
    static constexpr uint32_t TICKS_REQ = 2000ul / TIMER_PERIOD; 
    
    esp_timer_handle_t _timer;

    static void isr_handler_static(void* arg);

    void tick();
};