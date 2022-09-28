// epoll server

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define MAXLINE 100
#define LISTENQ 20
#define SERV_PORT 6666

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

void* send_timer(void *p) {
    int fd = *(int *)p;
    printf("[thread] fd =%d\n", fd);

    int wn = 0;
    for (int i = 0; i < 10; i++){
        const char *msg = "send_timer";
        wn = write(fd, msg, strlen(msg));
        printf("[thread] send size =%d\n", wn);
        sleep(1);
    }
    return NULL;
}

void catch_event(int sockfd, uint32_t fdevents){
    printf("fd = %d, event = 0x%X\n", sockfd, fdevents);
    if (fdevents & EPOLLERR) {
        printf("catch EPOLLERR\n");
    }
    if (fdevents & EPOLLHUP) {
        printf("catch EPOLLHUP\n");
    }
    if (fdevents & EPOLLRDHUP) {
        printf("catch EPOLLRDHUP\n");
    }
    if (fdevents & EPOLLIN) {
        printf("catch EPOLLIN\n");
    }
    if (fdevents & EPOLLOUT) {
        printf("catch EPOLLOUT\n");
    }
}

void catch_error(int sockfd){
    int error = 0;
    socklen_t len = sizeof(error);
    int code = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
    const char * err = NULL;
    if (errno != 0){
        err = strerror(errno);
    }else if (error != 0) {
        err = strerror(error);
    } else {
        err = "Unknown error";
    }
    printf("socket error, code=%d, errno=%d, error=%d, err=%s\n", code, errno, error, err);
}

int main() {
    int connfd = 0, sockfd, nfds;
    ssize_t n;
    char line[MAXLINE];
    socklen_t clilen;

    struct epoll_event ev, events[20];
    int epfd = epoll_create(256);
    if (epfd < 0) {
        printf("epoll create fail.");
        exit(-1);
    }

    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //把socket设置为非阻塞方式
    setnonblocking(listenfd);

    ev.data.fd = listenfd;
    ev.events = EPOLLIN;

    //注册epoll事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;

    char *local_addr = "127.0.0.1";
    inet_aton(local_addr, &(serveraddr.sin_addr));
    serveraddr.sin_port = htons(SERV_PORT);

    int flag = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) != 0){
        printf("set listen fd reuse fail.");
        exit(1);
    }

    bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listenfd, LISTENQ);

    pthread_t tid = 0;
    for (;;) {
        //等待epoll事件的发生
        printf("\nbegin epoll wait ...\n");
        nfds = epoll_wait(epfd, events, 20, -1);

        //处理所发生的所有事件
        for(int i = 0; i < nfds; ++i) {
            if(events[i].data.fd == listenfd){
                // 监听 socket
                connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clilen);
                if(connfd < 0) {
                    printf("connfd<0, errno=%d\n", errno);
                    exit(1);
                }
                setnonblocking(connfd);

                char *str = inet_ntoa(clientaddr.sin_addr);
                printf("connect from %s, fd=%d\n", str, connfd);

                ev.data.fd = connfd;
                ev.events = EPOLLIN | EPOLLRDHUP;
                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
            } else {
                sockfd = events[i].data.fd;
                uint32_t fdevents = events[i].events;
                catch_event(sockfd, fdevents);
                catch_error(sockfd);

                // EPOLLIN
                if(fdevents & EPOLLIN) {
                    if ( (sockfd = events[i].data.fd) < 0) continue;
                    if ( (n = read(sockfd, line, MAXLINE)) < 0) {
                        if (errno == ECONNRESET) {
                            close(sockfd);
                            events[i].data.fd = -1;
                            printf("errno fd\n");
                        } else {
                            printf("readline error \n");
                        }
                    } else if (n == 0) {
                        printf("n = 0, peer fd closed\n");

                        // 取消事件监听
                        ev.data.fd = sockfd;
                        ev.events ^= (EPOLLIN | EPOLLRDHUP);
                        epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
                    } else {
                        printf("fd: %d, recv msg: %s\n", sockfd, line);
                        //write(sockfd, "fuck", 4);
                        write(sockfd, line, n);

                        if (line[0] == '@'){
                            pthread_create(&tid, NULL, send_timer, &sockfd);
                        }
                    }
                }
            }
        }

        // sleep 200ms
        usleep(200*1000);
    }
}
