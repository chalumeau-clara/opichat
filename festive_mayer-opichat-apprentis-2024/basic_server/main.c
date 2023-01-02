#include "basic_server.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./basic_server SERVER_IP SERVER_PORT");
        return 10;
    }
    int server_socket = prepare_socket(argv[1], argv[2]);
    if (server_socket == -1)
        return 1;
    int server_socket_write = accept_client(server_socket);
    if (server_socket_write == 1)
        return 1;
    // close(server_socket);
    return 0;
}
