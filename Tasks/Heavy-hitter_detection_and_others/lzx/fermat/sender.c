#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libnet.h>

int main() {
    libnet_t *l;
    char *payload = "Hello, this is a test packet";
    int payload_s = strlen(payload);
    char errbuf[LIBNET_ERRBUF_SIZE];

    l = libnet_init(LIBNET_RAW4, "eth0", errbuf);
    if (l == NULL) {
        fprintf(stderr, "libnet_init() failed: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    libnet_ptag_t tcp = libnet_build_tcp(
        1234,   // 源端口
        80,     // 目的端口
        123456, // 序列号
        0,      // 确认号
        TH_PUSH,// 控制位
        29200,  // 窗口大小
        0,      // 校验和
        0,      // 紧急指针
        LIBNET_TCP_H + payload_s, // TCP头部长度
        (u_int8_t *)payload,      // 负载
        payload_s,                // 负载长度
        l,      // libnet句柄
        0       // 协议标记
    );
    libnet_ptag_t ip = libnet_build_ipv4(
        LIBNET_IPV4_H + LIBNET_TCP_H + payload_s, // IP数据包长度
        0,      // TOS
        242,    // ID
        0,      // 标志
        64,     // TTL
        IPPROTO_TCP, // 上层协议
        0,      // 校验和
        inet_addr("172.17.188.226"), // 源IP
        inet_addr("172.17.188.226"), // 目的IP
        NULL,   // 负载
        0,      // 负载长度
        l,      // libnet句柄
        0       // 协议标记
    );
    libnet_write(l);

    libnet_destroy(l);

    return 0;
}
