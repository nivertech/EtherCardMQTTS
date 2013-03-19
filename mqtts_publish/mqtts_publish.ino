
#include "EtherCardMQTTS.h"
#include <EtherCard.h>

#define BUF_SIZE      MQTTS_MAX_ETHER_LEN


byte mac[] = { 0x00, 0x04, 0xA3, 0x21, 0xCA, 0x38 };

uint8_t ip[] = { 192, 168, 1, 10 };
uint8_t dns[] = { 192, 168, 1, 1 };
uint8_t gateway[] = { 192, 168, 1, 1 };
uint8_t subnet[] = { 255, 255, 255, 0 };
uint8_t server[] = { 192, 168, 1, 1 };
byte Ethernet::buffer[BUF_SIZE];

// remote website name
char gateway_host[] PROGMEM = "test.mosquitto.org";

EtherCardMQTTS mqtts;


void setup(void)
{
    Serial.begin(9600);
    delay(1000);

    /* Check that the Ethernet controller exists */
    Serial.println("Initialising the Ethernet controller");
    if (ether.begin(sizeof Ethernet::buffer, mac, 8) == 0) {
        Serial.println( "Ethernet controller NOT initialised");
        while (true)
            /* MT */ ;
    }

    /* Setup IP address */
    ether.staticSetup(ip, gateway, dns);
    ether.printIp("My IP: ", ether.myip);
    ether.printIp("GW IP: ", ether.gwip);

    /* Set the MQTT-S server address */
    mqtts.setServer(server);

    /* Set the topic to publish to */
    mqtts.setTopicName(PSTR("test"));

    return;
}


void loop(void)
{
    word len = ether.packetReceive();
    mqtts.packetLoop(len);
}
