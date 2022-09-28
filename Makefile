CC ?= gcc

close1:
	$(CC) -g -Wall -o epollserver $@/epollserver.c
	$(CC) -g -Wall -o epollclient $@/epollclient.c
	$(CC) -g -Wall -o shardclient $@/shardclient.c

close2:
	$(CC) -g -Wall -o epollserver $@/epollserver.c
	$(CC) -g -Wall -o epollclient $@/epollclient.c


close3:
	$(CC) -g -Wall -o epollserver $@/epollserver.c
	$(CC) -g -Wall -o epollclient $@/epollclient.c

clear:
	rm -f epollserver
	rm -f epollclient
	rm -f shardclient

.PHONY: clear