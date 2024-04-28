#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

typedef struct HashBucket
{
    int count;
    int id;
} HashBucket;

int main() {
    int sockfd, new_sock, num_buckets;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int bytes_received = 0;

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    // 绑定套接字
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听，假设等待连接的最大数量为 10
    if (listen(sockfd, 10) == 0) {
        cout << "Listening..." << endl;
    } else {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // 接受连接
    new_sock = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);

    // 接收数据
    recv(new_sock, &num_buckets, sizeof(int), 0); // 先接收实例的数量
    cout << "Received number of buckets: " << num_buckets << endl;

    HashBucket received_buckets[num_buckets]; // 记录全部桶
    while (bytes_received < num_buckets * sizeof(HashBucket)) {
        int remaining_bytes = num_buckets * sizeof(HashBucket) - bytes_received;
        int bytes_to_receive = remaining_bytes > 1024 ? 1024 : remaining_bytes;
        bytes_received += recv(new_sock, (char *)received_buckets + bytes_received, bytes_to_receive, 0);

        // 接收"success"消息
        char success_msg[7];
        recv(new_sock, success_msg, 7, 0);
        success_msg[6] = '\0';
        cout << "Received: " << success_msg << endl;
    }

    // 将接收到的数据输出到 output.txt 文件
    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_buckets; i++) {
        fprintf(output_file, "Received count: %d, id: %d\n", received_buckets[i].count, received_buckets[i].id);
    }

    fclose(output_file);

    close(new_sock);
    close(sockfd);
    return 0;
}
