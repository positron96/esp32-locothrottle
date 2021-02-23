#include <Arduino.h>

#include <Keypad.h>

#include <U8g2lib.h>

// pins 2,7,6,4
const byte rows[] = { 14, 19, 23, 26 };
const int rowCount = sizeof(rows)/sizeof(rows[0]);
 
// pins 3,1,5
const byte cols[] = { 27, 12, 25};
const int colCount = sizeof(cols)/sizeof(cols[0]);

 
Keypad numpad( rows, cols, rowCount, colCount );

U8G2_PCD8544_84X48_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 16, /* data=*/ 17, /* cs=*/ 21, /* dc=*/ 18, /* reset=*/ 22); 
//U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 4, /* dc=*/ 5, /* reset=*/ 3); 


void keypadEventHandler(uint8_t key, KeyState st);

void setup() {
    Serial.begin(115200);

    u8g2.begin();

    numpad.addEventListener(keypadEventHandler);
}

void redraw() {
    u8g2.clearBuffer();	
    u8g2.setFont(u8g2_font_ncenB08_tr);	
    //u8g2.drawStr(0,10,"Hello World!");
    for(int i=0; i<3; i++)
        for(int j=0; j<4; j++) {
            KeyState st = numpad.getState(j*3+i);
            u8g2.drawGlyph(i*10, (j+1)*12, st==IDLE ? '.' : st==PRESSED ? 'P' : 'H');
        }
    u8g2.sendBuffer();
}

void loop() {
    static uint32_t frames=0;
    static uint32_t lastOutptTime=0;
    numpad.getKeys();
    frames++;
    if( (millis() - lastOutptTime)>1000) {
        lastOutptTime = millis();
        Serial.println(frames);
        frames=0;
    }
}

void keypadEventHandler(uint8_t key, KeyState st) {    
    Serial.printf("%d - %s\n", key, st==IDLE?"IDLE": st==PRESSED?"PRESSED":"HOLD" );
    redraw();
} 

