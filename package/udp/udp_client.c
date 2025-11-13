#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "udp_protocol.h"

#define SERVER_IP   "192.168.6.19"
#define DEV_ID      0x12345678

int main(void)
{
    int sock;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    uint8_t buf[1024];
    uint16_t seq = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("UDP client start, target %s:%d\n", SERVER_IP, UDP_PORT);

    while (1) {
        // === 发送心跳 ===
        uint8_t txbuf[256];
        udp_header_t *hdr = (udp_header_t*)txbuf;
        hdr->header[0] = PROTO_HEADER1;
        hdr->header[1] = PROTO_HEADER2;
        hdr->version   = 1;
        hdr->msg_type  = MSG_HEARTBEAT;
        hdr->dev_id    = htonl(DEV_ID);
        hdr->seq       = htons(seq++);
        hdr->length    = htons(0); // 无 payload

        int txlen = sizeof(udp_header_t);
        sendto(sock, txbuf, txlen, 0,
               (struct sockaddr*)&server_addr, addr_len);
        printf("Send heartbeat #%d\n", seq);

        // === 等待命令（非阻塞超时） ===
        struct timeval tv = {1, 0};  // 1s timeout
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);

        if (select(sock + 1, &rfds, NULL, NULL, &tv) > 0) {
            struct sockaddr_in from;
            socklen_t flen = sizeof(from);
            int n = recvfrom(sock, buf, sizeof(buf), 0,
                             (struct sockaddr*)&from, &flen);
            if (n > 0) {
                udp_header_t *rx = (udp_header_t*)buf;
                if (rx->msg_type == MSG_CMD) {
                    uint8_t cmd = buf[sizeof(udp_header_t)];
                    printf("Recv CMD=0x%02X from %s:%d\n",
                           cmd, inet_ntoa(from.sin_addr), ntohs(from.sin_port));

                    // 回复 ACK
                    uint8_t ackbuf[256];
                    udp_header_t *ack = (udp_header_t*)ackbuf;
                    ack->header[0] = PROTO_HEADER1;
                    ack->header[1] = PROTO_HEADER2;
                    ack->version   = 1;
                    ack->msg_type  = MSG_CMD_ACK;
                    ack->dev_id    = htonl(DEV_ID);
                    ack->seq       = rx->seq;
                    ack->length    = htons(1);
                    ackbuf[sizeof(udp_header_t)] = 0x00;  // 0=OK

                    int acklen = sizeof(udp_header_t) + 1;
                    sendto(sock, ackbuf, acklen, 0,
                           (struct sockaddr*)&from, flen);
                    printf("→ Send ACK for CMD=0x%02X\n", cmd);
                }
            }
        }

        sleep(2); // 2秒一个心跳
    }

    close(sock);
    return 0;
}
