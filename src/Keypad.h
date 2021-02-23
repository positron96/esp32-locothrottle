/**
 * A heavilty modified Keypad library.
 * @see https://github.com/Chris--A/Keypad
 */

#pragma once

#include <cstdint>
#include <cstdbool>

#include <functional>

typedef enum{ IDLE, PRESSED, HOLD } KeyState;

typedef struct {
    uint8_t key;
    KeyState state;
} KeypadEvent;

typedef struct {
    uint8_t rows;
    uint8_t columns;
    uint8_t num;
} KeypadSize;

typedef std::function<void(uint8_t, KeyState)> EventListener;// void (*EventListener)(uint8_t, KeyState);

const char NO_KEY = '\0';

class Keypad {
public:

	Keypad(const uint8_t *row, const uint8_t *col, const uint8_t numRows, const uint8_t numCols);
	bool getKeys();
    uint32_t scanKeys();
	KeyState getState(uint8_t idx);
	
	bool isPressed(uint8_t);
	void setDebounceTime(uint16_t);
	void setHoldTime(uint16_t);
	void addEventListener(EventListener);
	char waitForKey();
	bool keyStateChanged();
	uint8_t numKeys();

private:
	unsigned long lastScanTime;
    unsigned long lastHoldTime;

    const uint8_t *rowPins;
    const uint8_t *columnPins;
    KeypadSize sizeKpd;
	uint16_t debounceDelay;
	uint16_t holdDelay;
    uint32_t keysPressed, lastKeysPressed;
    uint32_t keysHeld;

	
	bool updateStates(uint32_t pressed);
	bool updateKeyState(uint8_t n, bool down);
	void transitionTo(uint8_t n, KeyState nextState);
	EventListener keypadEventListener;
};