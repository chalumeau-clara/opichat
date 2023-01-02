#include "epoll-server.h"

void print_epoll_socket(struct connection_t *connection)
{
    printf("List sock:\n");
    while (connection)
    {
        printf("socket id: %d\n", connection->client_socket);
        connection = connection->next;
    }
    printf("\n");
}
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./basic_server SERVER_IP SERVER_PORT");
        return 1;
    }
    int server_socket = prepare_socket(argv[1], argv[2]);
    if (server_socket == -1)
        return 1;
    // Create an epoll instance
    int epoll_instance = epoll_create1(0);

    // Initialize the event struct containing the flags and optional user data
    struct epoll_event event = { 0 };
    event.data.fd = server_socket;
    event.events = EPOLLIN;
    struct connection_t *connection = NULL;
    // Add the socket to the epoll instance's interest list.
    if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, server_socket, &event) == -1)
    {
        fprintf(stderr, "ADD SOCKET SERVER do not work\n");
        exit(1);
    }
    while (1)
    {
        struct epoll_event events[MAX_EVENTS];
        int events_count = epoll_wait(epoll_instance, events, MAX_EVENTS, -1);
        for (int event_idx = 0; event_idx < events_count; event_idx++)
        {
            // Client socket
            int client_socket = events[event_idx].data.fd;

            if (client_socket == server_socket)
            {
                connection =
                    accept_client(epoll_instance, server_socket, connection);
            }
            else // if (events[event_idx].events & EPOLLIN)
            {
                connection = communicate(client_socket, connection);
            }
        }
    }
    close(server_socket);
    return 0;
}
