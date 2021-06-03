/**
 * A heavilty modified Keypad library.
 * @see https://github.com/Chris--A/Keypad
 */

#pragma once

#include <cstdint>
#include <cstdbool>

#include <functional>
#include <etl/bitset.h>

typedef enum{ IDLE, PRESSED, HOLD } KeyState;

typedef struct {
    uint8_t key;
    KeyState state;
} KeyEvent;

using EventListener = std::function<void(uint8_t, KeyState)>;

template <class T, uint8_t N>
class KeysReader {
public:

    KeysReader() {
        setDebounceTime(10);
        setHoldTime(500);

        keypadEventListener = nullptr;
        lastScanTime = 0;
        lastHoldTime = 0;
    }

    bool getKeys() {
        // Limit how often the keypad is scanned. This makes the loop() run 10 times as fast.
        if ( (millis()-lastScanTime)>debounceDelay ) {
            static_cast<T*>(this)->scanKeys();
            lastScanTime = millis();
        }
        return keysPressed.any();
    }
	
	bool isPressed(uint8_t idx) {
        return keysPressed[idx];
    }

    // Minimum debounceDelay is 1 mS. Any lower *will* slow down the loop().
	void setDebounceTime(uint16_t debounce) {
        debounceDelay = debounce<1 ? 1 : debounce;
    }
	void setHoldTime(uint16_t hold) {
        holdDelay = hold;
    }

	void addEventListener(const EventListener &listener){
        keypadEventListener = listener;
    }

    uint8_t waitForKey() {
        while( ! getKeys() );	// Block everything while waiting for a keypress.
        return keysPressed.find_first(true);
    }

    KeyState getState(uint8_t idx) {
        if(keysHeld[idx]) return HOLD;
        else return keysPressed[idx] ? PRESSED : IDLE;
    }

	bool keyStateChanged() {
        return keysPressed != lastKeysPressed;
    }

	constexpr static uint8_t NUM = N;

protected:
	unsigned long lastScanTime;
    unsigned long lastHoldTime;

	uint16_t debounceDelay;
	uint16_t holdDelay;

    etl::bitset<N> keysPressed;
    etl::bitset<N> lastKeysPressed;
    etl::bitset<N> keysHeld;

    /*bool updateStates(...) {
        bool anyActivity = false;

        for (uint8_t i=0; i<N; i++) {
            if(updateKeyState(i, bitRead(pressed, i)) ) anyActivity = true;
        }
        lastKeysPressed = keysPressed;
        return anyActivity;
    }*/
	
	bool updateKeyState(uint8_t idx, bool down) {
        KeyState st = getState(idx);
        
        switch (st) {
            case IDLE:
                if (down) {
                    transitionTo(idx, PRESSED);
                    lastHoldTime = millis(); 
                    return true;
                }
                break;
            case PRESSED:
                if (down && (millis()-lastHoldTime)>holdDelay) {
                    transitionTo (idx, HOLD);
                    return true;
                } 
                if (!down) {
                    transitionTo (idx, IDLE);
                    return true;
                }
                break;
            case HOLD:
                if (!down) {
                    transitionTo (idx, IDLE);
                    return true;
                }
                break;
        }

        return false;
    }

	void transitionTo(uint8_t idx, KeyState nextState) {
        keysHeld.set(idx, nextState==HOLD);
        keysPressed.set(idx, nextState==PRESSED || nextState==HOLD);
        
        if ( keypadEventListener )  {
            keypadEventListener(idx, nextState);
        }
    }

	EventListener keypadEventListener;
};



template <uint8_t N>
class SimpleKeysReader: public KeysReader<SimpleKeysReader<N>, N> {
    using Base = KeysReader<SimpleKeysReader<N>, N>;
public:

    SimpleKeysReader(uint8_t const (&_pins)[N]): Base(), pins(_pins) {
        for(uint8_t i=0; i<N; i++) {
            pinMode(pins[i], INPUT_PULLUP);
        }
    };

private:
    const uint8_t *pins;

    void scanKeys() {        
        for(uint8_t i=0; i<N; i++) {
            uint8_t v = 1-digitalRead(pins[i]);
            this->updateKeyState(i, v!=0);
        }
    }

    friend bool Base::getKeys();
	
};