#include <Keypad.h>
#include <Arduino.h>

// <<constructor>> Allows custom keymap, pin configuration, and keypad sizes.
Keypad::Keypad(const uint8_t *row, const uint8_t *col, const uint8_t numRows, const uint8_t numCols) {
	rowPins = row;
	columnPins = col;
	sizeKpd.rows = numRows;
	sizeKpd.columns = numCols;
    sizeKpd.num = numRows*numCols;

	setDebounceTime(10);
	setHoldTime(500);

	keypadEventListener = nullptr;
	lastScanTime = 0;
	lastHoldTime = 0;
}

// Populate the key list.
bool Keypad::getKeys() {
	//bool keyActivity = false;

	// Limit how often the keypad is scanned. This makes the loop() run 10 times as fast.
	if ( (millis()-lastScanTime)>debounceDelay ) {
		
		//keyActivity = updateStates();
        updateStates(scanKeys());
		lastScanTime = millis();
	}

	return keysPressed != 0;//keyActivity;
}

// Private : Hardware scan
uint32_t Keypad::scanKeys() {
	uint32_t keys = 0;
	// Re-intialize the row pins. Allows sharing these pins with other hardware.
	for (byte r=0; r<sizeKpd.rows; r++) {
		pinMode(rowPins[r], INPUT_PULLUP);
	}

	// bitMap stores ALL the keys that are being pressed.
	for (byte c=0; c<sizeKpd.columns; c++) {
		pinMode(columnPins[c], OUTPUT);
		digitalWrite(columnPins[c], LOW);	
		for (byte r=0; r<sizeKpd.rows; r++) {
			bitWrite(keys, c + r*sizeKpd.columns, 1-digitalRead(rowPins[r]));
		}
		digitalWrite(columnPins[c],HIGH);
		pinMode(columnPins[c],INPUT);
	}
	return keys;
}

// Manage the list without rearranging the keys. Returns true if any keys on the list changed state.
bool Keypad::updateStates(uint32_t pressed) {
	bool anyActivity = false;

    for (uint8_t i=0; i<sizeKpd.num; i++) {
        if(updateKeyState(i, bitRead(pressed, i)) ) anyActivity = true;
    }
    lastKeysPressed = keysPressed;
	return anyActivity;
}

// Private
// This function is a state machine but is also used for debouncing the keys.
bool Keypad::updateKeyState(uint8_t idx, bool down) {
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

// New in 2.1
bool Keypad::isPressed(uint8_t idx) {
	return bitRead(keysPressed, idx)==1;
}

// New in 2.0
char Keypad::waitForKey() {
	char waitKey = NO_KEY;
	while( ! getKeys() );	// Block everything while waiting for a keypress.
	return waitKey;
}

KeyState Keypad::getState(uint8_t idx) {
	KeyState st;
	bool held = bitRead(keysHeld, idx);
    bool pressed = bitRead(keysPressed, idx);
    if(held) return HOLD;
    else return st = pressed ? PRESSED : IDLE;

}



// The end user can test for any changes in state before deciding
// if any variables, etc. needs to be updated in their code.
bool Keypad::keyStateChanged() {
	return keysPressed != lastKeysPressed;
}


// Minimum debounceDelay is 1 mS. Any lower *will* slow down the loop().
void Keypad::setDebounceTime(uint16_t debounce) {
	debounce<1 ? debounceDelay=1 : debounceDelay=debounce;
}

void Keypad::setHoldTime(uint16_t hold) {
    holdDelay = hold;
}

void Keypad::addEventListener(EventListener listener){
	keypadEventListener = listener;
}

void Keypad::transitionTo(uint8_t idx, KeyState nextState) {
    bitWrite( keysHeld, idx, (nextState==HOLD ? 1 : 0) );
    bitWrite( keysPressed, idx, ((nextState==PRESSED || nextState==HOLD) ? 1 : 0) );
	
	if ( keypadEventListener )  {
		keypadEventListener(idx, nextState);
	}
	
}

