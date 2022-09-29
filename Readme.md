不同方式关闭 socket 的演示代码。

- close1，演示 close() 的引用计数
- close2，演示接收缓冲区为空时调用 close
- close3，演示接收缓冲区不为空时调用 close

- shutdown1，演示 shutdown(fd, SHUT_RD)
- shutdown2，演示 shutdown(fd, SHUT_WR) 以及 SHUT_WR 后再 shutdown(fd, SHUT_RD)
- shutdown3，演示 shutdown(fd, SHUT_RDWR)