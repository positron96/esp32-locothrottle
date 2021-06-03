/**
 * A heavilty modified Keypad library.
 * @see https://github.com/Chris--A/Keypad
 */

#pragma once

#include <cstdint>
#include <cstdbool>

#include "KeysReader.h"

#include <functional>

template<uint8_t R, uint8_t C>
class Keypad: public KeysReader<Keypad<R,C>, R*C> {
	using Base = KeysReader<Keypad<R,C>, R*C>;
public:

	Keypad(const uint8_t *row, const uint8_t *col): Base() {
		rowPins = row;
		columnPins = col;
	}

private:

    const uint8_t *rowPins;
    const uint8_t *columnPins;

private:
	void scanKeys() {
		
		etl::bitset<Base::NUM> keys;
		// Re-intialize the row pins. Allows sharing these pins with other hardware.
		for (byte r=0; r<R; r++) {
			pinMode(rowPins[r], INPUT_PULLUP);
		}

		// bitMap stores ALL the keys that are being pressed.
		for (byte c=0; c<C; c++) {
			pinMode(columnPins[c], OUTPUT);
			digitalWrite(columnPins[c], LOW);	
			for (byte r=0; r<R; r++) {
				keys.set(c + r*C, 1-digitalRead(rowPins[r]) );
			}
			digitalWrite(columnPins[c],HIGH);
			pinMode(columnPins[c],INPUT);
		}

		for(int i=0; i<Base::NUM; i++) {
			this->updateKeyState(i, keys[i]);
		}
	}

	friend bool Base::getKeys();

	
};