#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct HashBucket
{
    int count;
    int id;
} HashBucket;

int main() {
    int sockfd;
    struct sockaddr_in dest_addr;

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置目标地址
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr);

    // 建立连接
    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    // 发送数据
    int num_buckets = 2000;
    HashBucket buckets[num_buckets];
    for (int i = 0; i < num_buckets; i++) {
        buckets[i].count = rand();
        buckets[i].id = rand();
    }

    send(sockfd, &num_buckets, sizeof(int), 0); // 先发送实例的数量

    int bytes_sent = 0;
    while (bytes_sent < num_buckets * sizeof(HashBucket)) {
        int remaining_bytes = num_buckets * sizeof(HashBucket) - bytes_sent;
        int bytes_to_send = remaining_bytes > 1024 ? 1024 : remaining_bytes;
        send(sockfd, (char *)buckets + bytes_sent, bytes_to_send, 0);
        bytes_sent += bytes_to_send;

        // 发送"success"消息
        send(sockfd, "success", strlen("success"), 0);
    }

    close(sockfd);
    return 0;
}
