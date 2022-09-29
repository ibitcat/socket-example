// epoll client

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void setnonblocking(int sock) {
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        printf("fcntl(sock,GETFL)");
        exit(1);
    }

    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0) {
        printf("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

void catch_error(int sockfd) {
    int error = 0;
    socklen_t len = sizeof(error);
    int code = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
    const char *err = NULL;
    if (errno != 0) {
        err = strerror(errno);
    } else if (error != 0) {
        err = strerror(error);
    } else {
        err = "Unknown error";
    }
    printf("socket error, code=%d, errno=%d, error=%d, err=%s\n", code, errno, error, err);
}

int main(int argc, char *argv[]) {
    printf("arg count = %d\n", argc);
    if (argc != 3) {
        printf("Usage: %s server_ip_address port\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[2]);
    int connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect_fd < 0) {
        perror("cannot create communication socket");
        return 1;
    }

    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    srv_addr.sin_port = htons(port);

    int ret = connect(connect_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (ret == -1) {
        perror("cannot connect to the server");
        close(connect_fd);
        return 1;
    }
    // 设置为非阻塞
    setnonblocking(connect_fd);
    printf("pid=%d, connect_fd=%d\n", getpid(), connect_fd);

    // sleep 2s，让接收缓冲区存点数据
    sleep(2);

    // 关闭写
    printf("shutdown write, fd=%d\n", connect_fd);
    shutdown(connect_fd, SHUT_WR);

    // 继续接收数据
    char rcv_buf[1024];
    while (1) {
        memset(rcv_buf, 0, 1024);
        int len = read(connect_fd, rcv_buf, 1024);
        printf("read msg from fd, fd=%d, len=%d, msg=%s\n", connect_fd, len, rcv_buf);

        if (len > 0) {
            if (rcv_buf[0] == '@') {
                // 关闭读，再收到对端的数据会返回 RST
                printf("shutdown read, fd=%d\n", connect_fd);
                shutdown(connect_fd, SHUT_RD);
            }
        } else {
            catch_error(connect_fd);
        }

        // sleep 500ms
        usleep(500 * 1000);
    }
    return 0;
}
