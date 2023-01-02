#include "epoll-server.h"

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
    {
        exit(1);
    }
    return sfd;
}

int prepare_socket(const char *ip, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
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

struct connection_t *accept_client(int epoll_instance, int server_socket,
                                   struct connection_t *connection)
{
    int socket_fd = accept(server_socket, NULL, NULL);
    if (socket_fd == -1)
        return connection;

    struct epoll_event event = { 0 };
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = socket_fd;
    // Non bloquant
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

    // Add the socket to the epoll instance's interest list.
    if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, socket_fd, &event) == -1)
    {
        fprintf(stderr, "Cannot add the socket to the epoll instance");
        close(socket_fd);
        return connection;
    }
    fprintf(stdout, "Client connected\n");
    connection = add_client(connection, socket_fd);
    return connection;
}

struct connection_t *communicate(int client_socket,
                                 struct connection_t *connection)
{
    // Initialize
    char buff[DEFAULT_BUFFER_SIZE];
    ssize_t n;
    while ((n = recv(client_socket, buff, DEFAULT_BUFFER_SIZE,
                     MSG_DONTWAIT | MSG_NOSIGNAL))
           > 0)
    {
        //					buff[n++] = '\0';
        // printf("Receive body : %s", buff);
        struct connection_t *tmp = connection;
        while (tmp)
        {
            send(tmp->client_socket, buff, n, MSG_NOSIGNAL | MSG_DONTWAIT);
            if (n == 0)
            {
                fprintf(stderr, "Client disconnected");
                connection = remove_client(connection, tmp->client_socket);
                fflush(stdout);
            }
            tmp = tmp->next;
            if (n == -1)
            {
                if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                    ;
                else
                {
                    printf("recv failed for socket = %d\n", client_socket);
                    connection = remove_client(connection, client_socket);
                }
            }
        }
    }
    return connection;
}
