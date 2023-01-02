#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <sys/types.h>

/**
 * \brief Contain all the information about all clients (linked list)
 */
struct connection_t
{
    int client_socket; /**< socket fd of the client */

    char login[10000]; /**<buffer containing loggin */

    char *buffer; /**< buffer containing all the data received by this client */

    ssize_t nb_read; /**< number of bytes read (also size of the buffer) */

    struct connection_t *next; /**< next connection_t for another client */

    int is_log; // if login == 1 otherwise 0

    char *ip; // ip of the client

    struct room_t *room;
};

struct socket_t
{
    int client_socket;

    int is_owner; // set 0 is not, 1 otherwise

    struct socket_t *next;
};

struct room_t
{
    char name[10000]; /**< name of the room */

    int owner; /** owner of the room**/

    struct socket_t *client_socket; /** all the client socket*/

    struct room_t *next;
};
/**
 * \brief Add a new client connection_t to the linked list connection
 *
 * \param connection: the connection_t linked list with all the clients
 *
 * \param client_socket: the client socket fd to add
 *
 * \return The connection_t linked list with the element added
 *
 * Add the new connection_t element with a head insert
 */
struct connection_t *add_client(struct connection_t *connection,
                                int client_socket, char *ip);

struct socket_t *add_socket(struct socket_t *socket, int client_socket);

struct room_t *add_room(struct room_t *socket, char *name, int client_socket);
/**
 * \brief Remove the client connection_t from the linked list connection
 *
 * \param connection: the connection_t linked list with all the clients
 *
 * \param client_socket: the client socket fd to remove
 *
 * \return The connection_t linked list with element removed
 *
 * Iterate over the linked list to find the right connection_t and remove it
 */
struct connection_t *remove_client(struct connection_t *connection,
                                   int client_socket);

struct socket_t *remove_socket(struct socket_t *socket, int client_socket);

struct room_t *remove_room(struct room_t *room, char *name);
/**
 * \brief Find the connection_t element where the socket is equal to client sock
 *
 * \param connection: the connection_t linked list with all the clients
 *
 * \param client_socket: the client socket to find
 *
 * \return The connection_t element of the specific client
 *
 * Iterate over the linked list until it finds the connection_t. If the client
 * is not in the linked list returns NULL
 */
struct connection_t *find_client(struct connection_t *connection,
                                 int client_socket);

/**
 * \brief Find the connection_t element where the socket is equal to client sock
 *
 * \param connection: the connection_t linked list with all the clients
 *
 * \param login: the login to find
 *
 * \return The connection_t element of the specific client
 *
 * Iterate over the linked list until it finds the connection_t. If the client
 * is not in the linked list returns NULL
 */
struct connection_t *find_login(struct connection_t *connection, char *login);
struct room_t *find_room(struct room_t *room, char *name);
// Return 0 if name already exist, 1 otherwise
int is_exist_room(struct room_t *room, char *name);
int is_exist_client(struct socket_t *socket, int socket_id);
int is_owner_room(struct room_t *room, int client_socket);

// Print all the room
void print_room(struct room_t *room);
#endif /* CONNECTION_H */
