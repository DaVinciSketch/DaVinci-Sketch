#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockfd, new_socket;
    struct sockaddr_in my_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024];

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 绑定地址
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(8080);
    bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));

    // 监听连接
    listen(sockfd, 5);

    // 接受连接
    new_socket = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);

    // 接收数据
    recv(new_socket, buffer, 1024, 0);
    printf("Received message: %s\n", buffer);

    close(new_socket);
    close(sockfd);
    return 0;
}
