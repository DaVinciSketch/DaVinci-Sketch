#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

struct HashBucket
{
    int count;
    char id[13];
};

int main() {
    // 创建套接字和连接服务器
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // 生成200个HashBucket实例
    const int num_buckets = 200;
    HashBucket buckets[num_buckets];
    for (int i = 0; i < num_buckets; i++) {
        buckets[i].count = rand() % 100; // 生成随机的count值
        for (int j = 0; j < 13; j++) {
            buckets[i].id[j] = rand() % 256; // 生成随机的8比特整数作为id的每个字节
        }
    }

    // 分批发送数据
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
