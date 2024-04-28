/*#include <stdio.h>
#include <pcap.h>

void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    // 处理接收到的数据包
    printf("Packet captured with length: %d\n", pkthdr->len);
    // 提取TCP头部
    printf("Payload:\n");
 
    for(int i = 0; i < pkthdr->len; i++)
    {
        printf("%02x ", packet[i]);
        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n\n");

    return;
}

int main() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    struct bpf_program fp;
    char filter_exp[] = "tcp port 81";
    bpf_u_int32 net;

    // 打开网络接口
    handle = pcap_open_live("ens33", BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device: %s\n", errbuf);
        return 2;
    }

    // 编译过滤器规则
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return 2;
    }

    // 设置过滤器规则
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return 2;
    }

    // 开始捕获数据包
    pcap_loop(handle, 0, packet_handler, NULL);

    // 关闭网络接口
    pcap_close(handle);

    return 0;
}*/
#include <stdio.h>
#include <pcap.h>

void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    // 处理接收到的数据包
    printf("Packet captured with length: %d\n", pkthdr->len);
}

int main() {
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    char filter_exp[] = "tcp port 5678";
    bpf_u_int32 net;

    // 打开网络接口
    handle = pcap_open_live("ens33", BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", "lo", errbuf);
        return 2;
    }

    // 编译过滤器规则
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return 2;
    }

    // 设置过滤器规则
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return 2;
    }

    // 开始捕获数据包
    pcap_loop(handle, 0, packet_handler, NULL);

    // 关闭网络接口
    pcap_close(handle);

    return 0;
}
