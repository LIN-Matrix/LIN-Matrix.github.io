#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "test.h"

#define TEST(c, ...) ((c) ? 1 : (t_error(#c " failed: " __VA_ARGS__), 0))
#define TESTE(c) (errno = 0, TEST(c, "errno = %s\n", strerror(errno)))

int main(void) {
    struct sockaddr_in sa = { .sin_family = AF_INET };
    int server_fd, client_fd;
    char buffer[100];

    // 创建 TCP Socket
    TESTE((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) >= 0);

    // 绑定地址和端口
    sa.sin_port = htons(8080); // 使用端口 8080
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 绑定到本地回环地址
    TESTE(bind(server_fd, (struct sockaddr *)&sa, sizeof(sa)) == 0);

    // 监听连接
    TESTE(listen(server_fd, 1) == 0);

    // 获取绑定的地址和端口
    socklen_t addrlen = sizeof(sa);
    TESTE(getsockname(server_fd, (struct sockaddr *)&sa, &addrlen) == 0);
    printf("Server listening on port %d\n", ntohs(sa.sin_port));

    // 创建客户端 Socket
    TESTE((client_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) >= 0);

    // 连接到服务器
    TESTE(connect(client_fd, (struct sockaddr *)&sa, sizeof(sa)) == 0);

    // 接受客户端连接
    int accepted_fd;
    TESTE((accepted_fd = accept(server_fd, (struct sockaddr *)&sa, &addrlen)) >= 0);

    // 客户端发送数据
    const char *message = "Hello, server!";
    TESTE(send(client_fd, message, strlen(message), 0) == strlen(message));

    // 服务器接收数据
    ssize_t received = recv(accepted_fd, buffer, sizeof(buffer), 0);
    TESTE(received >= 0);
    buffer[received] = '\0'; // 确保字符串以 null 结尾
    TEST(strcmp(buffer, message) == 0, "Received: '%s', Expected: '%s'\n", buffer, message);

    // 服务器发送响应
    const char *response = "Hello, client!";
    TESTE(send(accepted_fd, response, strlen(response), 0) == strlen(response));

    // 客户端接收响应
    received = recv(client_fd, buffer, sizeof(buffer), 0);
    TESTE(received >= 0);
    buffer[received] = '\0'; // 确保字符串以 null 结尾
    TEST(strcmp(buffer, response) == 0, "Received: '%s', Expected: '%s'\n", buffer, response);

    // 关闭 Socket
    close(accepted_fd);
    close(client_fd);
    close(server_fd);

    return t_status;
}