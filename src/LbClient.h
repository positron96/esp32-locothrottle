/** LocoNet over TCP client 
 * 
 * @see http://loconetovertcp.sourceforge.net/Protocol/LoconetOverTcp.html
*/

#pragma once

#include <LocoNet.h>

#include <WiFiClient.h>

#include <etl/queue.h>

#define FROM_HEX(c) (   ((c)>'9') ? ((c) &~ 0x20)-'A'+0xA : ((c)-'0')   )
#define TO_HEX1(b)  (   ((b)>9)   ?  (b)         +'A'-0xA : ((b)+'0')   )

#define LB_DEBUG

#ifdef LB_DEBUG
#define LB_LOGI(format, ...)  do{ log_printf(ARDUHAL_LOG_FORMAT(I, format), ##__VA_ARGS__); }while(0)
#define LB_LOGD(format, ...)  do{ log_printf(ARDUHAL_LOG_FORMAT(D, format), ##__VA_ARGS__); }while(0)
#else
#define LB_LOGI(...)
#define LB_LOGD(...) 
#endif


#define LBSERVER_TCP_PORT  1234


class LbClient: public LocoNetConsumer {
public:
    LbClient(LocoNetBus & bus) :  bus(bus) {
        bus.addConsumer(this);
    }

    void setServer(String host, uint16_t port) {
        LB_LOGI("%s:%d", host.c_str(), port);
        this->host = host;
        this->port = port;
    }

    void begin() {
        cli.connect(host.c_str(), port);
    }

    void end() {
        cli.stop();
    }

    void loop() {
        if(!cli) return;
        while(cli.available() > 0) {
            int v = cli.read();
            if(v==-1) break;
            if(v=='\r') continue;

            lbStr[lbPos] = v;
            if(v=='\n') {
                lbStr[lbPos] = ' '; lbStr[lbPos+1]=0;
                LB_LOGD("Processing string '%s'", lbStr);
                const static int RECV_LEN = strlen("RECEIVE");
                if(strncmp("RECEIVE", lbStr, RECV_LEN)==0) {
                    for(uint8_t i=RECV_LEN+1; i<=lbPos; i++) {
                        if(lbStr[i]==' ') {
                            uint8_t val = FROM_HEX(lbStr[i-2])<<4 | FROM_HEX(lbStr[i-1]);
                            //LB_LOGD("LbServer::loop adding byte %02x", val);
                            LnMsg *msg = lbBuf.addByte(val);
                            if(msg!=nullptr) {                                
                                //sendMessage(*msg); // echo

                                LN_STATUS ret = bus.broadcast(*msg, this);

                                //if(ret==LN_DONE) cli.println("SENT OK"); else
                                //if(ret==LN_RETRY_ERROR) cli.println("SENT ERROR LN_RETRY_ERROR");
                                break;
                            }
                        }
                    }           
                } else if( strncmp("SENT", lbStr, 4)==0 ) {
                    LB_LOGD("Sent result: %s", lbStr+4 );
                } else if( strncmp("VERSION", lbStr, 7)==0 ) {
                    LB_LOGD("Server version: %s", lbStr+7 );
                } else {
                    LB_LOGI("Got unknown line: %s", lbStr);
                }
                lbPos=0;
            } else {
                lbPos++;
            }

        }

        while(!txQueue.empty()) {
            sendMessage(txQueue.front());
            txQueue.pop();
        }


    }

    LN_STATUS onMessage(const lnMsg& msg) override {
        txQueue.push(msg);
        return LN_DONE;
    }

private:
    LocoNetBus & bus;    

    WiFiClient cli;

    String host;
    uint16_t port;

    const static int LB_BUF_SIZE = 100;
    char lbStr[LB_BUF_SIZE];
    int lbPos = 0;

    etl::queue<LnMsg, 5> txQueue;

    LocoNetMessageBuffer lbBuf;

    void sendMessage(const LnMsg &msg) {
        if(!cli) return;

        char ttt[LB_BUF_SIZE] = "SEND";
        uint t = strlen(ttt);
        uint8_t ln = msg.length();
        for(int j=0; j<ln; j++) {
            t += sprintf(ttt+t, " %02X", msg.data[j]);
        }
        LB_LOGD("Transmitting '%s'", ttt );
        t += sprintf(ttt+t, "\n");
        cli.write(ttt);
    }

};