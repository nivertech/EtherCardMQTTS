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

#ifndef EtherCardMQTTS_h
#define EtherCardMQTTS_h

#if ARDUINO >= 100
  #include <Arduino.h> // Arduino 1.0
#else
  #include <WProgram.h> // Arduino 0022
#endif


#define MQTTS_PORT_DEFAULT   (1883)
#define MQTTS_MAX_ETHER_LEN  (ETH_HEADER_LEN + IP_HEADER_LEN + UDP_HEADER_LEN + 255)

#define MQTTS_TYPE_ADVERTISE     (0x00)
#define MQTTS_TYPE_SEARCHGW      (0x01)
#define MQTTS_TYPE_GWINFO        (0x02)
#define MQTTS_TYPE_CONNECT       (0x04)
#define MQTTS_TYPE_CONNACK       (0x05)
#define MQTTS_TYPE_WILLTOPICREQ  (0x06)
#define MQTTS_TYPE_WILLTOPIC     (0x07)
#define MQTTS_TYPE_WILLMSGREQ    (0x08)
#define MQTTS_TYPE_WILLMSG       (0x09)
#define MQTTS_TYPE_REGISTER      (0x0A)
#define MQTTS_TYPE_REGACK        (0x0B)
#define MQTTS_TYPE_PUBLISH       (0x0C)
#define MQTTS_TYPE_PUBACK        (0x0D)
#define MQTTS_TYPE_PUBCOMP       (0x0E)
#define MQTTS_TYPE_PUBREC        (0x0F)
#define MQTTS_TYPE_PUBREL        (0x10)
#define MQTTS_TYPE_SUBSCRIBE     (0x12)
#define MQTTS_TYPE_SUBACK        (0x13)
#define MQTTS_TYPE_UNSUBSCRIBE   (0x14)
#define MQTTS_TYPE_UNSUBACK      (0x15)
#define MQTTS_TYPE_PINGREQ       (0x16)
#define MQTTS_TYPE_PINGRESP      (0x17)
#define MQTTS_TYPE_DISCONNECT    (0x18)
#define MQTTS_TYPE_WILLTOPICUPD  (0x1A)
#define MQTTS_TYPE_WILLTOPICRESP (0x1B)
#define MQTTS_TYPE_WILLMSGUPD    (0x1C)
#define MQTTS_TYPE_WILLMSGRESP   (0x1D)

#define MQTTS_FLAG_DUP     (0x1 << 7)
#define MQTTS_FLAG_QOS_0   (0x0 << 5)
#define MQTTS_FLAG_QOS_1   (0x1 << 5)
#define MQTTS_FLAG_QOS_2   (0x2 << 5)
#define MQTTS_FLAG_QOS_N1  (0x3 << 5)
#define MQTTS_FLAG_RETAIN  (0x1 << 4)
#define MQTTS_FLAG_WILL    (0x1 << 3)
#define MQTTS_FLAG_CLEAN   (0x1 << 2)

#define MQTTS_PROTOCOL_ID  (0x01)


class EtherCardMQTTS {
private:
  uint8_t server_ip[4];   // ip address of remote gateway
  word port;
  byte state;

public:
  EtherCardMQTTS();

  void connect(const uint8_t* server_ip, word port=MQTTS_PORT_DEFAULT);

  void processPacket(byte *buf, word len);
  word packetLoop(word plen);

};

typedef struct {
  byte length;
  byte type;
  byte flags;
  byte protocol_id;
  word duration;
  char client_id[21];
} connect_packet_t;

#endif
