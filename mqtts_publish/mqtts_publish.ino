#include <EtherCard.h>

#define gPB ether.buffer
#define MQTTS_PORT_H_V   (0x07)
#define MQTTS_PORT_L_V   (0x5B)
#define MQTTS_MAX_ETHER_LEN  (ETH_HEADER_LEN + IP_HEADER_LEN + UDP_HEADER_LEN + 255)

#define BUF_SIZE      MQTTS_MAX_ETHER_LEN


byte mac[] = { 0x00, 0x04, 0xA3, 0x21, 0xCA, 0x38 };

uint8_t ip[] = { 192, 168, 1, 10 };
uint8_t dns[] = { 192, 168, 1, 10 };
uint8_t gateway[] = { 192, 168, 1, 10 };
uint8_t subnet[] = { 255, 255, 255, 0 };
byte Ethernet::buffer[BUF_SIZE];


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

    /* Get a DHCP connection */
    ether.staticSetup(ip, gateway, dns);
    ether.printIp("Using static IP address: ", ether.myip);
    return;
}

void mqttsProcessPacket(byte *buf, word len)
{
    Serial.print("Recieved MQTTS packet. len=");
    Serial.println(len, DEC);
    Serial.println((char*)buf);
}

word mqttsPacketLoop(word plen)
{
    // Does the packet look like a MQTT-S UDP packet?
    if (plen > UDP_DATA_P &&
        gPB[ETH_TYPE_H_P] == ETHTYPE_IP_H_V &&
        gPB[ETH_TYPE_L_P] == ETHTYPE_IP_L_V &&
        gPB[IP_HEADER_LEN_VER_P] == 0x45 &&
        gPB[IP_PROTO_P] == IP_PROTO_UDP_V &&
        gPB[UDP_DST_PORT_H_P] == MQTTS_PORT_H_V &&
        gPB[UDP_DST_PORT_L_P] == MQTTS_PORT_L_V)
    {
        word len = ((word)gPB[UDP_LEN_H_P] << 8) + gPB[UDP_LEN_L_P] - UDP_HEADER_LEN;
        mqttsProcessPacket(&gPB[UDP_DATA_P], len);
        return 0;
    } else {
        // Otherwise process using the EtherCard packet handler
        return ether.packetLoop(plen);
    }
}

void loop(void)
{
    word len = ether.packetReceive();
    mqttsPacketLoop(len);
}
