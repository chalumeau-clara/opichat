#include "opichat_client.h"

int main(int argc, char *argv[])
{
    // Not enought argument
    if (argc != 3)
    {
        fprintf(stderr, "student: usage: /student <ip> <port>\n");
        return 1;
    }

    // Get server socket
    int server_socket = prepare_socket(argv[1], argv[2]);
    if (server_socket == -1)
        return 1;

    // Start managing client
    managing_threads(server_socket);

    return 0;
}
