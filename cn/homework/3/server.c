#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <error.h>


int main(int argc, char *argv[])
{
    if(argc < 2) {
        fprintf(stderr, "Usage: ./server <port>\n");
        exit(1);
    }
    int server_sock_listen, server_sock_data;
    struct sockaddr_in server_addr;
    char recv_msg[255];

    /* 创建socket */
    server_sock_listen = socket(AF_INET, SOCK_STREAM, 0);

    /* 指定服务器地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    //server_addr.sin_port = atoi(argv[1]);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY表示本机所有IP地址
    memset(&server_addr.sin_zero, 0, sizeof(server_addr.sin_zero)); //零填充
    /* 绑定socket与地址 */
    if(bind(server_sock_listen, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error: ");
        exit(1);
    }

    /* 监听socket */
    if(listen(server_sock_listen, 1) < 0) {
        perror("listen error: ");
    }
    fprintf(stdout, "Server is listening......\n");

    while(1) {
        if((server_sock_data = accept(server_sock_listen, NULL, NULL)) < 0) {
            perror("accept error: ");
        }
        fprintf(stdout, "Accept......\n");

        while(1) {
            /* 接收并显示消息 */
            memset(recv_msg, 0, sizeof(recv_msg)); //接收数组置零
            if(recv(server_sock_data, recv_msg, sizeof(recv_msg), 0) < 0) {
                perror("recv error: ");
            }
            if(strcmp(recv_msg, "bye\n") == 0) { // 如果收到"bye"
                fprintf(stdout, "Bye!\n"); // 打印
                break;
            }
            fprintf(stdout, "Recv: %s", recv_msg);

            // 发送消息
            fprintf(stdout, "Send: %s", recv_msg);
            if(send(server_sock_data, recv_msg, strlen(recv_msg), 0) < 0) {
                perror("send error: ");
            }
        }
        /* 关闭数据socket */
        close(server_sock_data);
        puts("");
    }

    /* 关闭监听socket */
    close(server_sock_listen);
    exit(0);
}
