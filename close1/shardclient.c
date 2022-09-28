#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

int main(int argc, char *argv[]) {
    printf("arg count = %d\n", argc);
    if(argc != 3) {
        printf("Usage: %s pid fd\n", argv[0]);
        return 1;
    }

    int len;
    int pid = atoi(argv[1]);
    int fd = atoi(argv[2]);

    int pidfd = syscall(SYS_pidfd_open, pid, 0);
    int newfd = syscall(SYS_pidfd_getfd, pidfd, fd, 0);
    printf("fd = %d\n", newfd);

    char snd_buf[1024];
    bzero(snd_buf, 1024);

    while(1){
        bzero(snd_buf, 1024);
        len = read(newfd, snd_buf, 1024);
        printf("Message from server: len=%d,  msg=%s\n", len, snd_buf);
    }

    sleep(1);
}