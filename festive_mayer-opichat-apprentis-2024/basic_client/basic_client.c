#include "basic_client.h"

#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int socket_id;

int create_and_connect(struct addrinfo *addrinfo)
{
    int socket_file_descriptor;
    struct addrinfo *addrs = addrinfo;
    for (; addrs != NULL; addrs = addrs->ai_next)
    {
        socket_file_descriptor =
            socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        if (socket_file_descriptor == -1)
            continue;
        if (connect(socket_file_descriptor, addrs->ai_addr, addrs->ai_addrlen)
            != -1)
            break; /* Success */
    }
    if (addrs == NULL)
        exit(1);
    return socket_file_descriptor;
}

int prepare_socket(const char *ip, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
    hints.ai_addrlen = 0;
    int err_code = getaddrinfo(ip, port, &hints, &result);
    if (err_code != 0)
    {
        freeaddrinfo(result);
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err_code));
        exit(1);
    }
    int socket_file_descriptor = create_and_connect(result);
    freeaddrinfo(result);
    if (socket_file_descriptor == 1)
    {
        // fprintf(stderr, "error while processing create_and_connect\n");
        exit(1);
    }
    return socket_file_descriptor;
}

void sigint_leave_handler()
{
    close(socket_id);
    // printf("Caught signal %d\n",s);
    exit(0);
}

void communicate(int server_socket)
{
    socket_id = server_socket;
    signal(SIGINT, sigint_leave_handler);
    while (1)
    {
        fprintf(stderr, "Enter your message:\n");
        char buff[10000];
        size_t buff_size = 10000;
        ssize_t stdin_size = read(0, buff, buff_size);
        if (stdin_size == -1)
        {
            // fprintf(stderr, "Can not read from client socket stdin");
            exit(1);
        }
        // printf("Buff: %d\n" , buff[stdin_size - 1]);
        if (stdin_size > 0 && buff[stdin_size - 1] != '\n')
        {
            close(server_socket);
            exit(0);
        }
        if (stdin_size == 0 && buff[0] != '\n')
        {
            close(server_socket);
            exit(0);
        }
        ssize_t send_size =
            send(server_socket, buff, stdin_size, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (send_size == -1)
        {
            printf("closed %d\n", close(server_socket));
            // fprintf(stderr, "Something wents wrong when send to server");
            exit(1);
        }
        ssize_t server_response_size = recv(server_socket, buff, buff_size, 0);
        if (server_response_size == -1)
        {
            fprintf(stderr, "Error while receiving server response");
            // printf("closed %d\n",close(server_socket));
            exit(1);
        }
        fprintf(stdout, "Server answered with: %s", buff);
    }
    // shutdown(server_socket, 2);
    close(server_socket);
}
