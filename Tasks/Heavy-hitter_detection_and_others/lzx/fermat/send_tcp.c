#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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
    const char *message = "Hello, receiver!";
    send(sockfd, message, strlen(message), 0);

    close(sockfd);
    return 0;
}
