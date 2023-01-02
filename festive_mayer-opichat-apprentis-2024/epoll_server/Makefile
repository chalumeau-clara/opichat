CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -fsanitize=address
TESTFLAG = -lcriterion
SRC = main.c connection.c utils/xalloc.c epoll-server.c

all: epoll_server

epoll_server: ${SRC} 
	gcc ${CFLAGS} $^ -o epoll_server


clean:
	rm -f epoll_server

