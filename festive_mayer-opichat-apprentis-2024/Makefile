CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -fsanitize=address
TESTFLAG = -lcriterion 
CLIENTSRC = opichat_client.c main_client.c  
SERVERSRC = opichat_server.c server_main.c connection.c ./utils/xalloc.c 
SRCTEST = tests/client_test.c 

all: opichat_client opichat_server

opichat_client: ${CLIENTSRC} 
	gcc ${CFLAGS} $^ -o $@

opichat_server: ${SERVERSRC} 
	gcc ${CFLAGS} $^ -o $@

opi_test: opichat_client.c ${SRCTEST}
	gcc ${CFLAGS} ${TESTFLAG} $^ -o testsuite
	
	
check: opi_test  
	./testsuite -j1 --verbose

clean:
	rm -f opichat_client testsuite

