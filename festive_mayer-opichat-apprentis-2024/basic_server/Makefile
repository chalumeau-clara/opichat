CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -fsanitize=address
TESTFLAG = -lcriterion
SRC = main.c basic_server.c

all: basic_server

basic_server: ${SRC} 
	gcc ${CFLAGS} $^ -o basic_server


clean:
	rm -f basic_server

