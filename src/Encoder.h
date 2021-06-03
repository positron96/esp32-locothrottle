#pragma once

#include <cstdint>
#include <Arduino.h>

extern "C" {
  #include "esp_timer.h"
}

class Encoder {
public:
    Encoder(uint8_t pin1, uint8_t pin2): 
        pin1(pin1), pin2(pin2)
    {
        
    }

    ~Encoder() { stop(); }

    void start() ;

    void stop();

private:
    uint8_t pin1, pin2;
    
    esp_timer_handle_t _timer;

    static void isr_handler_static(void* arg);

    void tick();
}