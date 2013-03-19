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

#define NIBBLE_TO_HEXCHAR(nibble) \
  ({ \
      byte n = nibble & 0xF; \
      (n < 0xA) ? ('0' + n) : ('A' + n - 10); \
  })


EtherCardMQTTS::EtherCardMQTTS()
{
    this->state = MQTTS_STATE_DISCONNECTED;
}


void EtherCardMQTTS::setServer(const uint8_t* server_ip, word port)
{
    memcpy(this->server_ip, server_ip, 4);
    this->port = port;
}

{

void EtherCardMQTTS::sendConnect(byte* buf)
{
    MQTTS_DEBUG_PRINTLN("Sending CONNECT");

    EtherCard::udpPrepare(this->port, this->server_ip, this->port);

    buf[1] = MQTTS_TYPE_CONNECT;
    buf[2] = MQTTS_FLAG_CLEAN;
    buf[3] = MQTTS_PROTOCOL_ID;
    buf[4] = 0x00;                // Duration High
    buf[5] = 0x00;                // Duration Low

    // Build up the client ID, appending the MAC address
    buf[6] = 'E';
    buf[7] = 'C';
    buf[8] = '-';
    buf[9] = 'M';
    buf[10] = 'Q';
    buf[11] = 'T';
    buf[12] = 'T';
    buf[13] = 'S';
    buf[14] = '-';
    for(int i=0; i<6; i++) {
        buf[15 + (2*i)] = NIBBLE_TO_HEXCHAR(ether.mymac[i] >> 4);
        buf[16 + (2*i)] = NIBBLE_TO_HEXCHAR(ether.mymac[i]);
    }

    buf[0] = 6 + 21;

    EtherCard::udpTransmit(buf[0]);

    Serial.println("CONNECT sent");

    this->state = MQTTS_STATE_WAIT_CONNACK;
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

    if (buf[1] == MQTTS_TYPE_CONNACK && this->state == MQTTS_STATE_WAIT_CONNACK) {
        if (buf[2] == 0) {
            this->sendRegister(buf);
        } else {
            MQTTS_DEBUG_PRINTLN("Error: non-zero return code");
            this->state = MQTTS_STATE_ERROR;
        }
    } else {
#ifdef MQTTS_DEBUG
        Serial.print("Unhanded type: 0x");
        Serial.print(buf[1], HEX);
        Serial.print(" while in state: ");
        Serial.println(this->state, DEC);
#endif
    }
}

word EtherCardMQTTS::packetLoop(word plen)
{
    byte* pb = ether.buffer;
    byte port_h = this->port >> 8;
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
        word len = word(pb[UDP_LEN_H_P], pb[UDP_LEN_L_P]) - UDP_HEADER_LEN;
        this->processPacket(&pb[UDP_DATA_P], len);
        return 0;
    } else if (this->state == MQTTS_STATE_DISCONNECTED && plen == 0 &&
              ether.isLinkUp() && !ether.clientWaitingGw())
    {
        // Try and connect
        this->sendConnect(ether.buffer + UDP_DATA_P);
    } else {
        // Otherwise process using the EtherCard packet handler
        return ether.packetLoop(plen);
    }
}
