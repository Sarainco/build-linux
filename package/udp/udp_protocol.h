#ifndef UDP_PROTOCOL_H
#define UDP_PROTOCOL_H

#include <stdint.h>

#define UDP_PORT        50000
#define PROTO_HEADER1   0xAA
#define PROTO_HEADER2   0x55

#define MSG_HEARTBEAT   0x01
#define MSG_CMD         0x10
#define MSG_CMD_ACK     0x11

#pragma pack(push, 1)
typedef struct {
    uint8_t  header[2];   // 0xAA, 0x55
    uint8_t  version;     // 1
    uint8_t  msg_type;    // message type
    uint32_t dev_id;      // unique device ID
    uint16_t seq;         // sequence number
    uint16_t length;      // payload length
} udp_header_t;
#pragma pack(pop)

static inline uint16_t crc16_ccitt(const uint8_t *buf, int len)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= (uint16_t)buf[i] << 8;
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

#endif
