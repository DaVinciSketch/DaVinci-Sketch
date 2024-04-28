#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>

using namespace std;

struct HashBucket
{
    int count;
    char id[13];
};

int main() {
    // 创建套接字和绑定端口
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // 接收数据
    const int num_buckets = 200;
    HashBucket received_buckets[num_buckets];
    int bytes_received = 0;
    while (bytes_received < num_buckets * sizeof(HashBucket)) {
        int remaining_bytes = num_buckets * sizeof(HashBucket) - bytes_received;
        int bytes_to_receive = remaining_bytes > 1024 ? 1024 : remaining_bytes;
        valread = recv(new_socket, (char *)received_buckets + bytes_received, bytes_to_receive, 0);
        bytes_received += valread;

        // 接收"success"消息
        char success_msg[7];
        valread = recv(new_socket, success_msg, 7, 0);
        success_msg[valread] = '\0';
        if (strcmp(success_msg, "success") != 0) {
            cerr << "Error: Unexpected message received" << endl;
            exit(EXIT_FAILURE);
        }
        cout << "Received: " << success_msg << endl;
    }

    // 将接收到的数据输出到output.txt文件
    ofstream output_file("output.txt");
    for (int i = 0; i < num_buckets; i++) {
        output_file << "Count: " << received_buckets[i].count << ", ID: ";
        for (int j = 0; j < 13; j++) {
            output_file << hex << (int)received_buckets[i].id[j];
        }
        output_file << endl;
    }
    output_file.close();

    close(new_socket);
    close(server_fd);
    return 0;
}
