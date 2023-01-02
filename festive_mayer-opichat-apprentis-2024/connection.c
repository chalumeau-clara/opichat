#include "connection.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils/xalloc.h"

struct connection_t *add_client(struct connection_t *connection,
                                int client_socket, char *ip)
{
    struct connection_t *new_connection = xmalloc(sizeof(struct connection_t));

    new_connection->client_socket = client_socket;
    new_connection->buffer = NULL;
    new_connection->nb_read = 0;
    new_connection->next = NULL;
    new_connection->is_log = 0;
    new_connection->ip = ip;
    if (connection)
        new_connection->room = connection->room;
    else
        new_connection->room = add_room(NULL, "", 0);
    struct connection_t *tmp = connection;
    if (connection != NULL)
    {
        while (tmp->next != NULL)
            tmp = tmp->next;
        tmp->next = new_connection;
    }
    else
        connection = new_connection;
    return connection;
}

struct socket_t *add_socket(struct socket_t *socket, int client_socket)
{
    struct socket_t *new_socket = xmalloc(sizeof(struct socket_t));

    new_socket->next = socket;
    new_socket->client_socket = client_socket;

    return new_socket;
}

void cp(char *str, size_t *i, char *sstr, size_t si)
{
    for (size_t m = 0; m < si; m++)
    {
        str[*i] = sstr[m];
        *i = *i + 1;
    }
}

struct room_t *add_room(struct room_t *room, char name[], int client_socket)
{
    struct room_t *new_room =
        xmalloc(sizeof(struct room_t) + sizeof(struct socket_t));

    size_t len = 0;
    cp(new_room->name, &len, name, strlen(name));
    new_room->name[len++] = '\n';
    new_room->name[len] = '\0';
    new_room->client_socket = NULL;
    new_room->client_socket =
        add_socket(new_room->client_socket, client_socket);
    new_room->owner = client_socket;
    new_room->next = NULL;
    struct room_t *tmp = room;
    if (room != NULL)
    {
        while (tmp->next != NULL)
            tmp = tmp->next;
        tmp->next = new_room;
    }
    else
        room = new_room;
    return room;
}
struct connection_t *remove_client(struct connection_t *connection,
                                   int client_socket)
{
    if (connection && connection->client_socket == client_socket)
    {
        struct connection_t *client_connection = connection->next;
        if (close(connection->client_socket) == -1)
            errx(1, "Failed to close socket");
        free(connection->buffer);
        free(connection);
        return client_connection;
    }

    struct connection_t *tmp = connection;
    while (tmp->next)
    {
        if (tmp->next->client_socket == client_socket)
        {
            struct connection_t *client_connection = tmp->next;
            tmp->next = client_connection->next;
            if (close(client_connection->client_socket) == -1)
                errx(1, "Failed to close socket");
            free(client_connection->buffer);
            free(client_connection);
            break;
        }
        tmp = tmp->next;
    }

    return connection;
}

struct socket_t *remove_socket(struct socket_t *socket, int client_socket)
{
    if (socket && socket->client_socket == client_socket)
    {
        struct socket_t *client_socket_client = socket->next;
        free(socket);
        return client_socket_client;
    }

    struct socket_t *tmp = socket;
    while (tmp->next)
    {
        if (tmp->next->client_socket == client_socket)
        {
            struct socket_t *client_socket_client = tmp->next;
            tmp->next = client_socket_client->next;
            free(client_socket_client);
            break;
        }
        tmp = tmp->next;
    }

    return socket;
}

void free_socket(struct socket_t *socket)
{
    while (socket)
        socket = remove_socket(socket, socket->client_socket);
}

struct room_t *remove_room(struct room_t *room, char *name)
{
    if (room && strstr(room->name, name) != NULL)
    {
        // struct room_t *tmp = room;
        // struct room_t *client_room = tmp;
        // room = room->next;
        // free_socket(client_room->client_socket);
        // free(client_room);
        return room->next;
    }

    struct room_t *tmp = room;
    while (tmp->next)
    {
        if (strstr(tmp->next->name, name) != NULL)
        {
            struct room_t *client_room = tmp->next;
            tmp->next = client_room->next;
            free_socket(client_room->client_socket);
            free(client_room);
            break;
        }
        tmp = tmp->next;
    }

    return room;
}
struct connection_t *find_client(struct connection_t *connection,
                                 int client_socket)
{
    while (connection != NULL && connection->client_socket != client_socket)
        connection = connection->next;

    return connection;
}

struct room_t *find_room(struct room_t *room, char *name)
{
    while (room != NULL)
    {
        if (strstr(room->name, name) != NULL)
            return room;
        room = room->next;
    }
    return room;
}

// return 1 if exist 0 otherwise
int is_exist_room(struct room_t *room, char *name)
{
    while (room != NULL)
    {
        if (strstr(room->name, name) != NULL)
            return 1;
        room = room->next;
    }
    return 0;
}

// return 1 in the room 0 otherwise
int is_exist_client(struct socket_t *socket, int socket_id)
{
    while (socket != NULL)
    {
        if (socket->client_socket == socket_id)
            return 1;
        socket = socket->next;
    }
    return 0;
}

int is_owner_room(struct room_t *room, int client_socket)
{
    if (room->owner == client_socket)
        return 1;
    return 0;
}

struct connection_t *find_login(struct connection_t *connection, char *login)
{
    while (connection != NULL
           && (connection->is_log == 0
               || strstr(connection->login, login) == NULL))
        connection = connection->next;
    return connection;
}

void print_room(struct room_t *room)
{
    printf("Print room\n");
    while (room != NULL)
    {
        printf("Name: %s", room->name);
        struct socket_t *tmp = room->client_socket;
        while (tmp != NULL)
        {
            printf("\tSocket:%d\n", tmp->client_socket);
            tmp = tmp->next;
        }
        room = room->next;
    }
}
