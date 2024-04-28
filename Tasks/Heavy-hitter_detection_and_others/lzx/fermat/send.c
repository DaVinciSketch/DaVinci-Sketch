#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libnet.h>

int main() {
    libnet_t *l;
    char errbuf[LIBNET_ERRBUF_SIZE];
    libnet_ptag_t ip_tag, tcp_tag;

    // 初始化libnet
    l = libnet_init(LIBNET_RAW4, NULL, errbuf);
    if (l == NULL) {
        fprintf(stderr, "libnet_init() failed: %s\n", errbuf);
        exit(EXIT_FAILURE);
    }

    // 构造一个IP数据包
    ip_tag = libnet_autobuild_ipv4(
        LIBNET_IPV4_H + LIBNET_TCP_H,  // IP数据包总长度
        IPPROTO_TCP,  // 上层协议
        libnet_name2addr4(l, "127.0.0.1", LIBNET_RESOLVE),  // 源IP
        libnet_name2addr4(l, "127.0.0.1", LIBNET_RESOLVE),  // 目的IP
        l  // libnet句柄
    );
    if (ip_tag == -1) {
        fprintf(stderr, "libnet_autobuild_ipv4() failed: %s\n", libnet_geterror(l));
        libnet_destroy(l);
        exit(EXIT_FAILURE);
    }

    // 构造一个TCP数据包
    tcp_tag = libnet_build_tcp(
        1234,  // 源端口
        5678,  // 目的端口
        libnet_get_prand(LIBNET_PRu32),  // 序列号
        libnet_get_prand(LIBNET_PRu32),  // 确认号
        TH_SYN,  // 控制位
        libnet_get_prand(LIBNET_PRu16),  // 窗口大小
        0,  // TCP校验和，0表示由libnet自动计算
        0,  // TCP紧急指针
        LIBNET_TCP_H,  // TCP数据包总长度
        NULL,  // TCP负载内容
        0,  // TCP负载内容长度
        l,  // libnet句柄
        0  // TCP数据包标记
    );
    if (tcp_tag == -1) {
        fprintf(stderr, "libnet_build_tcp() failed: %s\n", libnet_geterror(l));
        libnet_destroy(l);
        exit(EXIT_FAILURE);
    }

    // 发送数据包
    if (libnet_write(l) == -1) {
        fprintf(stderr, "libnet_write() failed: %s\n", libnet_geterror(l));
        libnet_destroy(l);
        exit(EXIT_FAILURE);
    }

    // 释放资源
    libnet_destroy(l);

    return 0;
}
