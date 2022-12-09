#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 1024

int main(int argc, char* argv[]) {
    int sockfd;
    char send_msg[MAXLINE], recv_msg[MAXLINE];
    char buf[MAXLINE];
    struct sockaddr_in server_addr;

    if(argc < 2) { // 参数数量不足 报错 退出
        fprintf(stderr, "Usage: ./client <ip_address> <port>\n");
        exit(1);
    }

    // 创建socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Create socket error: ");
        exit(1);
    }

    // 指定服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2])); // 指定端口
    //server_addr.sin_port = atoi(argv[2]);
    if(strcmp(argv[1], "localhost") == 0) {
        if(inet_aton("127.0.0.1", &server_addr.sin_addr) == 0) {
            fprintf(stderr, "inet_aton error for %s\n", argv[1]);
            exit(1);
        }
    }
    else {
        if(inet_aton(argv[1], &server_addr.sin_addr) == 0) {
            fprintf(stderr, "inet_aton error for %s\n", argv[1]);
            exit(1);
        }
    }
    /*struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(atoi(argv[3])); // 指定端口
    bind(sockfd, (struct sockaddr*)&client_addr, sizeof client_addr);*/
    memset(&server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    // 连接服务器
    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect error: ");
        exit(1);
    }

    while(1) {
        memset(send_msg, 0, sizeof(send_msg)); // 发送数组置0
        fprintf(stdout, "Myself: ");
        fgets(send_msg, MAXLINE, stdin); // 从标准输入上得到要发送的字符串
        // 发送消息
        if(send(sockfd, send_msg, strlen(send_msg), 0) < 0) {
            perror("send error: ");
        }
        // 发送出去之后再判断是不是"bye"
        if(strcmp(send_msg, "bye\n") == 0) {
            fprintf(stdout, "Bye!\n");
            break;
        }
        // 接收并显示消息
        memset(recv_msg, 0, sizeof(recv_msg)); // 接收数组置0
        if((recv(sockfd, recv_msg, MAXLINE, 0)) < 0) {
            perror("recv error: ");
        }
        fprintf(stdout, "Server: %s", recv_msg);
    }
    if(close(sockfd) < 0) {
        perror("close error: ");
    }
    exit(0);
}
