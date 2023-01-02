#include "basic_server.h"

#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int create_and_bind(struct addrinfo *addrinfo)
{
    struct addrinfo *addrs = addrinfo;
    int sfd;
    for (; addrs != NULL; addrs = addrs->ai_next)
    {
        sfd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        if (sfd == -1)
            continue;

        int optval = 1;
        int err =
            setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
        if (err == -1)
            continue;
        if (bind(sfd, addrs->ai_addr, addrs->ai_addrlen) == 0)
            break; /* Success */
        close(sfd);
    }

    if (addrs == NULL)
        exit(1);
    return sfd;
}

int prepare_socket(const char *ip, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int err_code = getaddrinfo(ip, port, &hints, &result);
    if (err_code != 0)
    {
        freeaddrinfo(result);
        fprintf(stderr, "getaddrinfo: %s\n", strerror(err_code));
        exit(1);
    }
    int socket_file_descriptor = create_and_bind(result);
    freeaddrinfo(result);
    if (socket_file_descriptor == 1)
    {
        fprintf(stderr, "error while processing create_and_connect\n");
        exit(1);
    }
    err_code = listen(socket_file_descriptor, SOCK_STREAM);
    if (err_code != 0)
        exit(1);
    return socket_file_descriptor;
}

int accept_client(int socket)
{
    struct sockaddr_in sock_received;
    int socket_size = sizeof(sock_received);
    // memset(&sock_received, 0, sizeof(struct sockaddr_in));
    int socket_fd = 1;
    while (1)
    {
        socket_fd = accept(socket, (struct sockaddr *)&sock_received,
                           (socklen_t *)&socket_size);
        if (socket_fd == -1)
        {
            fprintf(stderr, "Error socket_fd response");
            exit(1);
        }
        pid_t pid = fork();
        if (pid == -1)
        {
            fprintf(stderr, "Error PID response");
            exit(1);
        }
        if (pid == 0)
        {
            close(socket);
            communicate(socket_fd);
        }
        fprintf(stdout, "Client connected\n");
    }
    return socket_fd;
}
int socket_id;
void sigint_leave_handler()
{
    close(socket_id);
    printf("Server disconnected\n");
    exit(0);
}

void cp_str(char *str, size_t *i, char *sstr, size_t si)
{
    for (size_t m = 0; m < si; m++)
    {
        str[*i] = sstr[m];
        *i = *i + 1;
    }
}
void communicate(int server_socket)
{
    socket_id = server_socket;
    signal(SIGPIPE, sigint_leave_handler);
    ssize_t server_response_size = 0;
    char buff_tot[10000] = { 0 };
    size_t buff_index_tot = 0;

    while (1)
    {
        char buff[10000] = { 0 };
        size_t buff_size = 10000;
        while ((server_response_size = recv(server_socket, buff, buff_size,
                                            MSG_DONTWAIT | MSG_NOSIGNAL))
               > 0)
        {
            cp_str(buff_tot, &buff_index_tot, buff, server_response_size);
        }
        if (server_response_size == 0)
        {
            printf("Client disconnect\n");
            close(server_socket);
            exit(0);
        }
        if (strstr(buff_tot, "\n"))
        {
            ssize_t send_size = send(server_socket, buff_tot, buff_index_tot,
                                     MSG_DONTWAIT | MSG_NOSIGNAL);
            printf("size %ld\n", buff_index_tot + 1);
            buff_index_tot = 0;
            if (send_size == -1)
            {
                printf("Failed to send\n");
                close(server_socket);
                exit(0);
            }
            fprintf(stdout, "Received Body: %s", buff);
        }
        /*
                        if (!strstr(buff, "\n"))
                        {
                                printf("Client disconnect without newline\n");
                                close(server_socket);
                                exit(0);
                        }
        */
    }
    // shutdown(server_socket, 2);
    close(server_socket);
}
