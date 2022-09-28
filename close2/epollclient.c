// epoll client

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

void setnonblocking(int sock){
    int opts;
    opts = fcntl(sock, F_GETFL);
    if(opts < 0){
        printf("fcntl(sock,GETFL)");
        exit(1);
    }

    opts = opts | O_NONBLOCK;
    if(fcntl(sock, F_SETFL, opts) < 0){
        printf("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    printf("arg count = %d\n", argc);
    if(argc != 3) {
        printf("Usage: %s server_ip_address port\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[2]);
    int connect_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(connect_fd < 0){
        perror("cannot create communication socket");
        return 1;
    }

    struct sockaddr_in srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    srv_addr.sin_port = htons(port);

    int ret = connect(connect_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if(ret == -1) {
        perror("cannot connect to the server");
        close(connect_fd);
        return 1;
    }
    //setnonblocking(connect_fd);
    printf("pid=%d, connect_fd=%d\n", getpid(), connect_fd);

    char snd_buf[1024];
    memset(snd_buf, 0, 1024);

    int len = 0;
    while(1){
        write(STDOUT_FILENO, "input message:", 14);
        bzero(snd_buf, 1024);
        len = read(STDIN_FILENO, snd_buf, 1024);
        if(len > 0){
            write(connect_fd, snd_buf, len);
        }
        if(snd_buf[0] == '@'){
            break;
        }
    }
    printf("close connect fd, fd=%d\n", connect_fd);
    close(connect_fd);

    // hold 住不要退出进程
    while (1){
        sleep(1);
    }
    return 0;
}
