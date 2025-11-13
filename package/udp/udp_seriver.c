#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "udp_protocol.h"

int main(void)
{
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    uint8_t buf[1024];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    printf("UDP server listening on port %d...\n", UDP_PORT);

    while (1) {
        int n = recvfrom(sock, buf, sizeof(buf), 0,
                         (struct sockaddr*)&client_addr, &addr_len);
        if (n <= 0)
            continue;

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        uint16_t port = ntohs(client_addr.sin_port);

        udp_header_t *hdr = (udp_header_t*)buf;
        if (hdr->header[0] != PROTO_HEADER1 || hdr->header[1] != PROTO_HEADER2)
            continue;

        printf("Recv msg from %s:%d  DevID=0x%08X  Type=0x%02X  Len=%d\n",
               ip_str, port, ntohl(hdr->dev_id), hdr->msg_type, ntohs(hdr->length));

        if (hdr->msg_type == MSG_HEARTBEAT) {
            printf("→ Heartbeat OK\n");

            // 发送命令包回去（测试）
            uint8_t txbuf[256];
            udp_header_t *tx = (udp_header_t*)txbuf;
            tx->header[0] = PROTO_HEADER1;
            tx->header[1] = PROTO_HEADER2;
            tx->version   = 1;
            tx->msg_type  = MSG_CMD;
            tx->dev_id    = hdr->dev_id;
            tx->seq       = htons(1);
            tx->length    = htons(1);
            txbuf[sizeof(udp_header_t)] = 0x01;  // CMD: START

            int txlen = sizeof(udp_header_t) + 1;
            sendto(sock, txbuf, txlen, 0,
                   (struct sockaddr*)&client_addr, addr_len);
            printf("→ Send CMD (0x01) to %s:%d\n", ip_str, port);
        }
    }

    close(sock);
    return 0;
}
