/*
 * Copyright (c) 2013, Nicholas Humfrey.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "EtherCardMQTTS.h"
#include <EtherCard.h>


// Uncomment to enable debugging of EtherCardMQTTS
#define MQTTS_DEBUG   1

#ifdef MQTTS_DEBUG
#define MQTTS_DEBUG_PRINTLN(str) Serial.println(str);
#else
#define MQTTS_DEBUG_PRINTLN(str)
#endif


EtherCardMQTTS::EtherCardMQTTS()
{
}

void EtherCardMQTTS::connect(const uint8_t* server_ip, word port)
{
    connect_packet_t *packet = (connect_packet_t*)(ether.buffer + UDP_DATA_P);

    memcpy(this->server_ip, server_ip, 4);
    this->port = port;

#ifdef MQTTS_DEBUG
    EtherCard::printIp("Server: ", this->server_ip);
#endif

    EtherCard::udpPrepare(this->port, this->server_ip, this->port);

    packet->type = MQTTS_TYPE_CONNECT;
    packet->flags = MQTTS_FLAG_CLEAN;
    packet->protocol_id = MQTTS_PROTOCOL_ID;
    packet->duration = 0;
    packet->client_id[0] = 'E';
    packet->client_id[1] = 'C';
    packet->client_id[2] = '-';
    packet->client_id[3] = 'M';
    packet->client_id[4] = 'Q';
    packet->client_id[5] = 'T';
    packet->client_id[6] = 'T';
    packet->client_id[7] = 'S';
    packet->client_id[8] = '-';

    // FIXME: improve this
    for(int i=0; i<6; i++) {
        String hex = String(ether.mymac[i], HEX);
        if (hex.length() == 2) {
            packet->client_id[9+(i*2)+0] = hex[0];
            packet->client_id[9+(i*2)+1] = hex[1];
        } else {
            packet->client_id[9+(i*2)+0] = '0';
            packet->client_id[9+(i*2)+1] = hex[0];
        }
    }

    packet->length = 6 + 21;

    EtherCard::udpTransmit(packet->length);

    Serial.println("CONNECT sent");
}


void EtherCardMQTTS::processPacket(byte *buf, word len)
{
    MQTTS_DEBUG_PRINTLN("Recieved MQTTS packet");

    if (len < 2) {
        MQTTS_DEBUG_PRINTLN("Error: MQTT-S packet is too short");
        return;
    } else if (len > 0xFF) {
        MQTTS_DEBUG_PRINTLN("Error: MQTT-S packet is too big");
        return;
    } else if (len != buf[0]) {
        MQTTS_DEBUG_PRINTLN("Error: MQTT-S length header does not match packet length");
        return;
    }

    Serial.print("Packet type: 0x");
    Serial.println(buf[1], HEX);
}

word EtherCardMQTTS::packetLoop(word plen)
{
    byte* pb = ether.buffer;
    byte port_h = (this->port >> 8);
    byte port_l = this->port & 0xFF;

    // Does the packet look like a MQTT-S UDP packet?
    if (plen > UDP_DATA_P &&
        pb[ETH_TYPE_H_P] == ETHTYPE_IP_H_V &&
        pb[ETH_TYPE_L_P] == ETHTYPE_IP_L_V &&
        pb[IP_HEADER_LEN_VER_P] == 0x45 &&
        pb[IP_PROTO_P] == IP_PROTO_UDP_V &&
        pb[UDP_DST_PORT_H_P] == port_h &&
        pb[UDP_DST_PORT_L_P] == port_l)
    {
        word len = ((word)pb[UDP_LEN_H_P] << 8) + pb[UDP_LEN_L_P] - UDP_HEADER_LEN;
        this->processPacket(&pb[UDP_DATA_P], len);
        return 0;
    } else {
        // Otherwise process using the EtherCard packet handler
        return ether.packetLoop(plen);
    }
}

