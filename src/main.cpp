#include <Arduino.h>

#include <KeysReader.h>
#include <Keypad.h>

#include <U8g2lib.h>

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>

#include <ESP32Encoder.h>


// pins 2,7,6,4
const byte rows[] = { 14, 19, 23, 26 };
const int rowCount = sizeof(rows)/sizeof(rows[0]);
 
// pins 3,1,5
const byte cols[] = { 27, 12, 25};
const int colCount = sizeof(cols)/sizeof(cols[0]);

 
Keypad<rowCount,colCount> numpad( rows, cols );

SimpleKeysReader<1> keys({4});

ESP32Encoder encoder;

U8G2_PCD8544_84X48_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 16, /* data=*/ 17, /* cs=*/ 21, /* dc=*/ 18, /* reset=*/ 22); 
//U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 4, /* dc=*/ 5, /* reset=*/ 3); 


/*
#include <LocoNet.h>
#include <LocoNetThrottle.h>
#include "LbClient.h"
LocoNetBus bus;
LocoNetDispatcher parser(&bus);
LocoNetThrottle thr(parser, 0,0,  ESP.getEfuseMac()&0xFFFF );
LbClient client(bus);
*/


void keypadEventHandler(uint8_t key, KeyState st);
void buttonEventHandler(uint8_t key, KeyState st);
void criticalError(String err);
void redraw();

void setup() {
    Serial.begin(115200);

    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setFontPosTop();
    u8g2.setFontMode(1);

    numpad.addEventListener(keypadEventHandler);
    keys.addEventListener(buttonEventHandler);

    ESP32Encoder::useInternalWeakPullResistors=UP;
	encoder.attachHalfQuad(2,15);

    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(300); // 5 min
	if ( !wifiManager.autoConnect("RailThrottle AP") ) {
        criticalError("No WiFi");
	}

/*
    MDNS.begin("ESP32Throttle");
	MDNS.setInstanceName("ESP32ThrottleInst");

    // hack: look for WiThrottle, but connect to LbServer
    //on server: MDNS.addService("withrottle","tcp", port);
    int nrOfServices = MDNS.queryService("withrottle","tcp");
   
    if (nrOfServices == 0) {
        criticalError("No LbServer");
    } else {
        client.setServer( MDNS.IP(0).toString(), LBSERVER_TCP_PORT);//MDNS.port(0) );
        client.begin();
    }

    //parser.onPacket(CALLBACK_FOR_ALL_OPCODES, [](const lnMsg *rxPacket) {        
        //char tmp[100];
        //formatMsg(*rxPacket, tmp, sizeof(tmp));
        //Serial.printf("onPacket: %s\n", tmp);
    //});

    thr.onAddressChange( [](LocoNetThrottle *t, uint16_t addr, uint16_t old) {
        redraw();
    });        
    thr.onSpeedChange( [](LocoNetThrottle *, uint8_t) {
        redraw();
    });    
    thr.onDirectionChange( [](LocoNetThrottle *, uint8_t) {
        redraw();
    });    
    thr.onFunctionChange( [](LocoNetThrottle *, uint8_t, bool) {
        redraw();
    });    
    thr.onSlotStateChange( [](LocoNetThrottle *, uint8_t) {
        redraw();
    });    
    thr.onError( [](LocoNetThrottle *, TH_ERROR err) {
        Serial.print("onError: ");
        Serial.println( thr.getErrorStr(err) );
    });    
    thr.onThrottleStateChange( [](LocoNetThrottle *, TH_STATE, TH_STATE) {
        redraw();
    });

    thr.setAddress(13);
*/

}

void criticalError(String err) {
    Serial.println(err);
    u8g2.clearBuffer();	
    u8g2.drawStr(0, 20, err.c_str());
    u8g2.sendBuffer();
    while(1) delay(1);
    //delay(1000);
	//ESP.restart();
}

void redraw() {
    u8g2.clearBuffer();	
    u8g2.setFont(u8g2_font_6x10_tr);	

    //u8g2.drawStr(0,10,"Hello World!");
    /*for(int i=0; i<3; i++)
        for(int j=0; j<4; j++) {
            KeyState st = numpad.getState(j*3+i);
            u8g2.drawGlyph(i*10, (j+1)*12, st==IDLE ? '.' : st==PRESSED ? 'P' : 'H');
        }*/
    
    String t = "Addr: ";  t += (int32_t)encoder.getCount();
    u8g2.drawStr(0, 2, t.c_str() );
    /*
    u8g2.drawStr(0, 12, thr.getStateStr( thr.getState() ));
    t = thr.getDirection()==0 ? "F" : "B";
    t += thr.getSpeed();
    u8g2.drawStr(0, 22, t.c_str() );

    t = "F:";
    for(int i=0; i<10; i++) {
        if( thr.getFunction(i)!=0) t+= i;
    }
    u8g2.drawStr(0, 32, t.c_str() );
*/
    u8g2.sendBuffer();
}

void loop() {
    //static uint32_t frames=0;
    //static uint32_t lastOutptTime=0;
    numpad.getKeys();
    keys.getKeys();
    static int32_t enc;
    if(enc!=encoder.getCount() ) {
        enc = encoder.getCount();
        Serial.printf("%d\n", enc);
        redraw();
    }
    //client.loop();
    /*frames++;
    if( (millis() - lastOutptTime)>1000) {
        lastOutptTime = millis();
        Serial.println(frames);
        frames=0;
    }*/
}

constexpr uint8_t KEY_1 = 0;
constexpr uint8_t KEY_2 = 1;
constexpr uint8_t KEY_3 = 2;
constexpr uint8_t KEY_4 = 3;
constexpr uint8_t KEY_5 = 4;
constexpr uint8_t KEY_6 = 5;
constexpr uint8_t KEY_7 = 6;
constexpr uint8_t KEY_8 = 7;
constexpr uint8_t KEY_9 = 8;
constexpr uint8_t KEY_STAR = 9; 
constexpr uint8_t KEY_0 = 10;
constexpr uint8_t KEY_DASH = 11;

void keypadEventHandler(uint8_t key, KeyState st) {    
    //Serial.printf("%d - %s\n", key, st==IDLE?"IDLE": st==PRESSED?"PRESSED":"HOLD" );
    //redraw();

    if(st==IDLE) {
        switch(key) {
            /*case KEY_STAR:
                thr.setAddress(13);
                break;
            case KEY_DASH: 
                if(thr.getState()==TH_ST_FREE) {
                    thr.acquireAddress(); 
                } else thr.releaseAddress();  
                break;
            case KEY_1: thr.setSpeed(0); break;
            case KEY_2: thr.setSpeed(10); break;
            case KEY_3: thr.setDirection( thr.getDirection()==0?1:0 ); break;
            
            case KEY_4: thr.setFunction(0, thr.getFunction(0)==0?1:0 ); break;
            case KEY_5: thr.setFunction(1, thr.getFunction(1)==0?1:0 ); break;

            case KEY_7: requestSwitch(&bus, 10, 1, 1); break;
            case KEY_8: requestSwitch(&bus, 10, 0, 1); break;
            default: break;*/
        }
    }
} 



void buttonEventHandler(uint8_t key, KeyState st) {    
    Serial.printf("Evt %d state %d\n", key, (int)st);
}
