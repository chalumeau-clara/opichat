CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -fsanitize=address
TESTFLAG = -lcriterion
SRC = main.c basic_client.c

all: basic_client

basic_client: ${SRC} 
	gcc ${CFLAGS} $^ -o basic_client


check:  
	./python_echo.py 0.0.0.0 8000

clean:
	rm -f basic_client

