#include "opichat_server.h"

// Load the sstr of si size un str at i index
void cp_str(char *str, size_t *i, char *sstr, size_t si)
{
    for (size_t m = 0; m < si; m++)
    {
        str[*i] = sstr[m];
        *i = *i + 1;
    }
}

// Return 1 if command has parameters, 0 otherwise
int has_parameters(char *buffer)
{
    if ((strcmp(buffer, "SEND-ROOM\n") == 0)
        || (strcmp(buffer, "SEND-ROOM") == 0))
    {
        return 1;
    }
    return 0;
}

// Print all epoll socket in connection instance
void print_epoll_socket(struct connection_t *connection)
{
    printf("List sock:\n");
    while (connection)
    {
        printf("socket id: %d\n", connection->client_socket);
        printf("login: %s\n", connection->login);
        connection = connection->next;
    }
    printf("\n");
}

int is_alpha_numerical_character(char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '1' && c <= '9'))
    {
        return 1;
    }
    return 0;
}

// Return 1 if the parameters send is valid, 0 otherwise
int is_valid_send_parameters(char *rest)
{
    if (rest[0] != 'U')
        return 0;
    else if (rest[1] != 's')
        return 0;
    else if (rest[2] != 'e')
        return 0;
    else if (rest[3] != 'r')
        return 0;
    else if (rest[4] != '=')
        return 0;
    else if (is_alpha_numerical_character(rest[5]) == 0)
        return 0;
    return 1;
}

// load the response from the response buffer
void load_response(char *response_buffer, char **payload_length_buff,
                   char **status_code_buff, char **command_buff,
                   char **parameters_buff, char **payload_buff,
                   size_t *payload_length_size, size_t *status_code_size,
                   size_t *command_size, size_t *parameters_size,
                   size_t *payload_size)
{
    char *rest = NULL;
    char *temp = response_buffer;

    // Get payload size
    *payload_length_buff = strtok_r(temp, "\n", &rest);
    *payload_length_size = strlen(*payload_length_buff);

    // Get status code
    *status_code_buff = strtok_r(NULL, "\n", &rest);
    *status_code_size = strlen(*status_code_buff);

    // Get command
    *command_buff = strtok_r(NULL, "\n", &rest);
    *command_size = strlen(*command_buff);

    // Get parameters
    size_t parameters_index = 0;

    if (has_parameters(*command_buff) == 1
        || is_valid_send_parameters(rest) == 1)
    {
        char *token = NULL;
        *parameters_buff = malloc(10000);
        for (token = strtok_r(NULL, "\n", &rest); rest[0] != '\n';
             token = strtok_r(NULL, "\n", &rest))
        {
            cp_str(*parameters_buff, &parameters_index, token, strlen(token));
            (*parameters_buff)[parameters_index] = '\n';
            parameters_index++;
        }
        cp_str(*parameters_buff, &parameters_index, token, strlen(token));
        (*parameters_buff)[parameters_index] = '\n';
        parameters_index++;
        *parameters_size = strlen(*parameters_buff);
    }

    // Get payloads
    *payload_size = 0;
    if (atoi(*payload_length_buff) != 0)
    {
        *payload_buff = strtok_r(NULL, "\n", &rest);
        *payload_size = strlen(*payload_buff);
    }
}

// Concatenate all the request into one buffer
void get_request(char *buff_request, char *buff_command, char *buff_parameters,
                 char *buff_payload, char *buff_status_code,
                 size_t size_payload, size_t size_parameters,
                 size_t *size_request)
{
    // Size payload
    char str_payload[3] = { '\0' };
    // if (size_payload > 0)
    //  size_payload--;
    sprintf(str_payload, "%ld", size_payload);
    cp_str(buff_request, size_request, str_payload, strlen(str_payload));
    buff_request[*size_request] = '\n';

    // Status Code for request
    *size_request = *size_request + 1;
    cp_str(buff_request, size_request, buff_status_code,
           strlen(buff_status_code));
    buff_request[*size_request] = '\n';

    // Command
    *size_request = *size_request + 1;
    size_t size_command = strlen(buff_command);
    cp_str(buff_request, size_request, buff_command, size_command);
    buff_request[*size_request] = '\n';
    *size_request = *size_request + 1;

    // Parameters
    cp_str(buff_request, size_request, buff_parameters, size_parameters);
    buff_request[*size_request] = '\n';
    *size_request = *size_request + 1;

    // Payload
    cp_str(buff_request, size_request, buff_payload, size_payload);
    buff_request[*size_request] = '\0';
}

// get the user in the parameter buffer
char *get_user(char *parameters)
{
    char *rest = NULL;
    char *token = NULL;
    token = strtok_r(parameters, "\n", &rest);
    while (strstr(token, "User") == NULL)
    {
        token = strtok_r(NULL, "\n", &rest);
    }
    char *new = NULL;
    new = strtok_r(NULL, "=", &token);
    new = strtok_r(NULL, "\n", &token);
    return new;
}

// get the room in the parameter buffer
char *get_room(char *parameters)
{
    char *rest = NULL;
    char *token = NULL;
    token = strtok_r(parameters, "\n", &rest);
    while (strstr(token, "Room") == NULL)
    {
        token = strtok_r(NULL, "\n", &rest);
    }
    char *new = NULL;
    new = strtok_r(NULL, "=", &token);
    new = strtok_r(NULL, "\n", &token);
    return new;
}

// get the last occurrence of a \n in the string
int get_last_occurrence(char *string)
{
    int index = 0;
    for (size_t i = 0; i != strlen(string); i++)
    {
        if (string[i] == '\n')
            index = i;
    }
    return index++;
}

// Verify str is an alphanumeric string
int is_string_alnum(char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!isalnum(str[i]) && str[i] != '\n')
            return 0;
    }
    return 1;
}

// Manage all the command
struct connection_t *handle_command(char *buff_command, char *buff_parameters,
                                    char *buff_payload, size_t size_payload,
                                    size_t size_parameters, int client_socket,
                                    struct connection_t *connection)
{
    char buff_request[10000];
    size_t size_request = 0;
    // char payload_length_buff[10000];
    char status_code_buff[10000];
    char command_buff[10000];
    char parameters_buff[10000];
    char temp_buff_parameters[10000];
    char payload_buff[10000];
    int err = 0;

    // PING
    if (strstr(buff_command, "PING") != NULL)
    {
        size_t len = 0;
        cp_str(status_code_buff, &len, "1", 1);
        len = 0;
        cp_str(command_buff, &len, "PING", 4);
        len = 0;
        cp_str(payload_buff, &len, "PONG\n", 5);
    }

    // LOGIN
    if (strstr(buff_command, "LOGIN") != NULL)
    {
        buff_payload[size_payload] = '\0';
        struct connection_t *client = find_login(connection, buff_payload);
        // Error no alphanumerical
        if (is_string_alnum(buff_payload) == 0)
        {
            size_t len = 0;
            cp_str(status_code_buff, &len, "3", 1);
            len = 0;
            cp_str(command_buff, &len, "LOGIN", 5);
            len = 0;
            cp_str(payload_buff, &len, "Bad username\n", 13);
        }
        // Without error
        else if (client == NULL)
        {
            size_t len = 0;
            cp_str(status_code_buff, &len, "1", 1);
            len = 0;
            cp_str(command_buff, &len, "LOGIN", 5);
            len = 0;
            cp_str(payload_buff, &len, "Logged in\n", 10);
            struct connection_t *client =
                find_client(connection, client_socket);
            len = 0;
            cp_str(client->login, &len, buff_payload, size_payload);
            client->login[len++] = '\n';
            client->login[len] = '\0';
            client->is_log = 1;
        }
        // Error duplicate username
        else
        {
            size_t len = 0;
            cp_str(status_code_buff, &len, "3", 1);
            len = 0;
            cp_str(command_buff, &len, "LOGIN", 5);
            len = 0;
            cp_str(payload_buff, &len, "Duplicate username\n", 19);
        }
    }

    // LIST-USERS
    if (strstr(buff_command, "LIST-USERS") != NULL)
    {
        size_t len = 0;
        cp_str(status_code_buff, &len, "1", 1);
        len = 0;
        cp_str(command_buff, &len, "LIST-USERS", 11);
        len = 0;
        struct connection_t *temp = connection;
        printf("payload %s\n", payload_buff);
        while (temp)
        {
            if (temp->is_log == 1)
                cp_str(payload_buff, &len, temp->login, strlen(temp->login));
            printf("payload %s\n", payload_buff);
            temp = temp->next;
        }
    }

    // BROADCAST
    if (strstr(buff_command, "BROADCAST") != NULL)
    {
        size_t len = 0;

        // Notification
        cp_str(status_code_buff, &len, "2", 1);
        len = 0;
        cp_str(command_buff, &len, "BROADCAST", 9);
        len = 0;
        cp_str(payload_buff, &len, buff_payload, size_payload);
        payload_buff[size_payload] = '\0';
        len = 0;
        cp_str(parameters_buff, &len, "From=", 5);
        struct connection_t *client = find_client(connection, client_socket);
        if (client->is_log == 1)
            cp_str(parameters_buff, &len, client->login, strlen(client->login));
        else
            cp_str(parameters_buff, &len, "<Anonymous>\n\0", 13);
        char notif_buffer[10000];
        get_request(notif_buffer, command_buff, parameters_buff, payload_buff,
                    status_code_buff, strlen(payload_buff),
                    strlen(parameters_buff), &size_request);

        printf("> Message out:\n%s\n", notif_buffer);

        // Send to all the connected client
        struct connection_t *all_clients = connection;
        while (all_clients != NULL)
        {
            if (client_socket != all_clients->client_socket)
            {
                ssize_t err = send(all_clients->client_socket, notif_buffer,
                                   size_request, MSG_NOSIGNAL | MSG_DONTWAIT);
                if (err == -1)
                    printf("something went wrong with send syscall\n");
            }
            all_clients = all_clients->next;
        }
        // Init buffer
        err = 0;
        memset(payload_buff, '\0', 10000);
        memset(status_code_buff, '\0', 10000);
        memset(parameters_buff, '\0', 10000);
        memset(buff_request, '\0', 10000);
        len = 0;
        size_request = 0;
        cp_str(status_code_buff, &len, "1", 1);
    }
    // SEND-DM
    if (strstr(buff_command, "SEND-DM") != NULL)
    {
        size_t len = 0;
        cp_str(command_buff, &len, "SEND-DM", 7);
        len = 0;
        // Error missing parameter
        if (size_parameters == 0)
        {
            cp_str(status_code_buff, &len, "3", 1);
            len = 0;
            cp_str(parameters_buff, &len, buff_payload, strlen(buff_payload));
            cp_str(parameters_buff, &len, "\n", 1);
            len = 0;
            cp_str(payload_buff, &len, "Missing parameter\n", 18);
            get_request(buff_request, command_buff, parameters_buff,
                        payload_buff, status_code_buff, strlen(payload_buff),
                        strlen(parameters_buff), &size_request);
            printf("> Message out:\n%s\n\n", buff_request);
            send(client_socket, buff_request, size_request,
                 MSG_NOSIGNAL | MSG_DONTWAIT);
            err = 1;
        }
        else
        {
            // Notification
            cp_str(status_code_buff, &len, "2", 1);
            len = 0;

            cp_str(payload_buff, &len, buff_payload, size_payload);
            payload_buff[size_payload] = '\0';

            len = 0;
            cp_str(parameters_buff, &len, buff_parameters,
                   strlen(buff_parameters));
            len = get_last_occurrence(buff_parameters);
            parameters_buff[++len] = '\0';
            // FROM
            struct connection_t *client =
                find_client(connection, client_socket);
            cp_str(parameters_buff, &len, "From=", 5);
            if (client->is_log == 1)
                cp_str(parameters_buff, &len, client->login,
                       strlen(client->login) + 1);
            else
                cp_str(parameters_buff, &len, "<Anonymous>\n\0", 13);
            // Users
            len = 0;

            cp_str(temp_buff_parameters, &len, buff_parameters,
                   strlen(buff_parameters) + 1);
            len = get_last_occurrence(buff_parameters);
            temp_buff_parameters[++len] = '\0';
            char *user = get_user(buff_parameters);
            client = find_login(connection, user);
            if (client == NULL || strstr(client->login, user) == NULL)
            {
                // Error
                len = 0;
                cp_str(status_code_buff, &len, "3", 1);
                len = 0;
                if (strstr(temp_buff_parameters, "User=") == NULL)
                    cp_str(payload_buff, &len, "Missing parameter\n", 18);
                else
                    cp_str(payload_buff, &len, "User not found\n", 15);
                get_request(buff_request, command_buff, temp_buff_parameters,
                            payload_buff, status_code_buff,
                            strlen(payload_buff), strlen(temp_buff_parameters),
                            &size_request);
                send(client_socket, buff_request, size_request,
                     MSG_NOSIGNAL | MSG_DONTWAIT);
                err = 1;
            }
            else
            {
                get_request(buff_request, command_buff, parameters_buff,
                            buff_payload, status_code_buff, size_payload,
                            strlen(parameters_buff), &size_request);

                printf("> Message out:\n%s\n", buff_request);
                send(client->client_socket, buff_request, size_request,
                     MSG_NOSIGNAL | MSG_DONTWAIT);
            }
            memset(payload_buff, '\0', 10000);
            memset(status_code_buff, '\0', 10000);
            memset(command_buff, '\0', 10000);
            // memset(parameters_buff, '\0', 10000);
            memset(buff_request, '\0', 10000);
            size_request = 0;
            // RESPONSE
            if (err == 0)
            {
                len = 0;
                cp_str(status_code_buff, &len, "1", 1);
                len = 0;
                cp_str(command_buff, &len, "SEND-DM", 7);
                len = 0;
                cp_str(parameters_buff, &len, temp_buff_parameters,
                       strlen(temp_buff_parameters) + 1);
                len = get_last_occurrence(temp_buff_parameters);
                parameters_buff[++len] = '\0';
                len = 0;
                // cp_str(payload_buff, &len, buff_payload,
                // strlen(buff_payload));
            }
        }
    }

    // CREATE ROOM
    if (strstr(buff_command, "CREATE-ROOM") != NULL)
    {
        buff_payload[size_payload] = '\0';
        // Error no alphanumerical
        if (is_string_alnum(buff_payload) == 0)
        {
            size_t len = 0;
            cp_str(status_code_buff, &len, "3", 1);
            len = 0;
            cp_str(command_buff, &len, "CREATE-ROOM", 11);
            len = 0;
            cp_str(payload_buff, &len, "Bad room name\n", 14);
        }
        // Error duplicate room
        else if (is_exist_room(connection->room, buff_payload) == 1)
        {
            size_t len = 0;
            cp_str(status_code_buff, &len, "3", 1);
            len = 0;
            cp_str(command_buff, &len, "CREATE-ROOM", 11);
            len = 0;
            cp_str(payload_buff, &len, "Duplicate room name\n", 20);
        }

        // No error
        else
        {
            size_t len = 0;
            cp_str(status_code_buff, &len, "1", 1);
            len = 0;
            cp_str(command_buff, &len, "CREATE-ROOM", 11);
            len = 0;
            cp_str(payload_buff, &len, "Room created\n", 13);
            len = 0;
            connection->room =
                add_room(connection->room, buff_payload, client_socket);
        }
    }

    // LIST-ROOM
    if (strstr(buff_command, "LIST-ROOM") != NULL)
    {
        size_t len = 0;
        cp_str(status_code_buff, &len, "1", 1);
        len = 0;
        cp_str(command_buff, &len, "LIST-ROOMS", 10);
        len = 0;
        struct room_t *room = connection->room->next;
        while (room != NULL)
        {
            cp_str(payload_buff, &len, room->name, strlen(room->name));
            room = room->next;
        }
    }

    // JOIN-ROOM
    if (strstr(buff_command, "JOIN-ROOM") != NULL)
    {
        buff_payload[size_payload] = '\0';
        struct room_t *room = find_room(connection->room, buff_payload);
        if (room == NULL)
        {
            // ERROR
            size_t len = 0;
            cp_str(status_code_buff, &len, "3", 1);
            len = 0;
            cp_str(command_buff, &len, "JOIN-ROOM", 9);
            len = 0;
            cp_str(payload_buff, &len, "Room not found\n", 15);
        }
        else
        {
            size_t len = 0;
            cp_str(status_code_buff, &len, "1", 1);
            len = 0;
            cp_str(command_buff, &len, "JOIN-ROOM", 9);
            len = 0;
            cp_str(payload_buff, &len, "Room joined\n", 12);
            room->client_socket =
                add_socket(room->client_socket, client_socket);
        }
    }
    // LEAVE-ROOM
    if (strstr(buff_command, "LEAVE-ROOM") != NULL)
    {
        buff_payload[size_payload] = '\0';
        struct room_t *room = find_room(connection->room, buff_payload);
        if (room == NULL)
        {
            // ERROR
            size_t len = 0;
            cp_str(status_code_buff, &len, "3", 1);
            len = 0;
            cp_str(command_buff, &len, "LEAVE-ROOM", 10);
            len = 0;
            cp_str(payload_buff, &len, "Room not found\n", 15);
        }
        else
        {
            // No error
            size_t len = 0;
            cp_str(status_code_buff, &len, "1", 1);
            len = 0;
            cp_str(command_buff, &len, "LEAVE-ROOM", 10);
            len = 0;
            cp_str(payload_buff, &len, "Room left\n", 10);
            room->client_socket =
                remove_socket(room->client_socket, client_socket);
        }
    }

    // DELETE ROOM
    if (strstr(buff_command, "DELETE-ROOM") != NULL)
    {
        buff_payload[size_payload] = '\0';
        struct room_t *room = find_room(connection->room, buff_payload);
        if (room == NULL)
        {
            // ERROR
            size_t len = 0;
            cp_str(status_code_buff, &len, "3", 1);
            len = 0;
            cp_str(command_buff, &len, "DELETE-ROOM", 11);
            len = 0;
            cp_str(payload_buff, &len, "Room not found\n", 15);
        }
        // Error duplicate room
        else if (is_owner_room(room, client_socket) == 0)
        {
            size_t len = 0;
            cp_str(status_code_buff, &len, "3", 1);
            len = 0;
            cp_str(command_buff, &len, "DELETE-ROOM", 11);
            len = 0;
            cp_str(payload_buff, &len, "Unauthorized\n", 13);
        }

        // No error
        else
        {
            size_t len = 0;
            cp_str(status_code_buff, &len, "1", 1);
            len = 0;
            cp_str(command_buff, &len, "DELETE-ROOM", 11);
            len = 0;
            cp_str(payload_buff, &len, "Room deleted\n", 13);
            len = 0;
            connection->room = remove_room(connection->room, buff_payload);
        }
    }

    // SEND ROOM
    if (strstr(buff_command, "SEND-ROOM") != NULL)
    {
        char *name = get_room(buff_parameters);
        struct room_t *room = find_room(connection->room, name);
        if (room == NULL)
        {
            // ERROR
            size_t len = 0;
            cp_str(status_code_buff, &len, "3", 1);
            len = 0;
            cp_str(command_buff, &len, "SEND-ROOM", 9);
            len = 0;
            cp_str(payload_buff, &len, "Room not found\n", 15);
            len = 0;
            cp_str(parameters_buff, &len, buff_parameters,
                   strlen(buff_parameters));
            len = get_last_occurrence(buff_parameters);
            parameters_buff[++len] = '\0';
        }
        else
        {
            size_t len = 0;

            // Notification
            cp_str(status_code_buff, &len, "2", 1);
            len = 0;
            cp_str(command_buff, &len, "SEND-ROOM", 9);
            len = 0;
            cp_str(payload_buff, &len, buff_payload, size_payload);
            payload_buff[size_payload] = '\0';
            len = 0;
            struct connection_t *client =
                find_client(connection, client_socket);

            // FROM @ ROOM
            len = 0;
            cp_str(parameters_buff, &len, "Room=", 5);
            cp_str(parameters_buff, &len, name, strlen(name));
            parameters_buff[len++] = '\n';
            cp_str(parameters_buff, &len, "From=", 5);
            printf("log num: %ld\n", strlen(client->login));
            if (client->is_log == 1)
                cp_str(parameters_buff, &len, client->login,
                       strlen(client->login));
            else
                cp_str(parameters_buff, &len, "<Anonymous>\n\0", 13);
            len = 0;
            char notif_buffer[10000];
            get_request(notif_buffer, command_buff, parameters_buff,
                        payload_buff, status_code_buff, strlen(payload_buff),
                        strlen(parameters_buff), &size_request);
            printf("> Message out:\n%s\n", notif_buffer);

            // Sens to all client connected in the given room
            struct socket_t *all_clients = room->client_socket;
            while (all_clients != NULL)
            {
                if (client_socket != all_clients->client_socket)
                {
                    ssize_t err =
                        send(all_clients->client_socket, notif_buffer,
                             size_request, MSG_NOSIGNAL | MSG_DONTWAIT);
                    if (err == -1)
                        printf("something went wrong with send syscall\n");
                }
                all_clients = all_clients->next;
            }
            err = 0;
            memset(payload_buff, '\0', 10000);
            memset(status_code_buff, '\0', 10000);
            // memset(parameters_buff, '\0', 10000);
            memset(buff_request, '\0', 10000);
            len = 0;
            size_request = 0;

            // RESPONSE
            cp_str(status_code_buff, &len, "1", 1);
            len = 0;
            cp_str(parameters_buff, &len, "Room=", 5);
            cp_str(parameters_buff, &len, name, strlen(name));
            cp_str(parameters_buff, &len, "\n", 1);
            parameters_buff[len] = '\0';
        }
    }
    // PROFILE
    if (strstr(buff_command, "PROFILE") != NULL)
    {
        size_t len = 0;
        cp_str(status_code_buff, &len, "1", 1);
        len = 0;
        cp_str(command_buff, &len, "PROFILE", 7);
        len = 0;

        // Username
        cp_str(payload_buff, &len, "Username: ", 10);
        struct connection_t *client = find_client(connection, client_socket);
        if (client->is_log == 1)
            cp_str(payload_buff, &len, client->login, strlen(client->login));
        else
            cp_str(payload_buff, &len, "<Anonymous>\n", 12);

        // IP
        cp_str(payload_buff, &len, "IP: ", 4);
        cp_str(payload_buff, &len, client->ip, strlen(client->ip));
        cp_str(payload_buff, &len, "\n", 1);

        // ROOMS
        struct room_t *room = connection->room;
        cp_str(payload_buff, &len, "Rooms:\n", 7);

        while (room != NULL)
        {
            if (is_exist_client(room->client_socket, client_socket) == 1)
                cp_str(payload_buff, &len, room->name, strlen(room->name));
            room = room->next;
        }
        cp_str(payload_buff, &len, "\0", 1);
        printf("parameters len: %ld", strlen(parameters_buff));
    }

    // If no error
    if (err == 0)
    {
        get_request(buff_request, command_buff, parameters_buff, payload_buff,
                    status_code_buff, strlen(payload_buff),
                    strlen(parameters_buff), &size_request);
        printf("> Message out:\n%s\n", buff_request);
        send(client_socket, buff_request, size_request,
             MSG_NOSIGNAL | MSG_DONTWAIT);
    }

    // Init buffer
    memset(payload_buff, '\0', 10000);
    memset(status_code_buff, '\0', 10000);
    memset(command_buff, '\0', 10000);
    memset(parameters_buff, '\0', 10000);
    memset(buff_request, '\0', 10000);
    memset(temp_buff_parameters, '\0', 10000);
    return connection;
}

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
                                   struct connection_t *connection, char *ip)
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
    connection = add_client(connection, socket_fd, ip);
    return connection;
}

struct connection_t *communicate(int client_socket,
                                 struct connection_t *connection)
{
    // Initialize
    char response_buff[DEFAULT_BUFFER_SIZE];
    ssize_t n;
    while ((n = recv(client_socket, response_buff, DEFAULT_BUFFER_SIZE,
                     MSG_DONTWAIT | MSG_NOSIGNAL))
           > 0)
    {
        char *payload_length_buff = NULL;
        size_t payload_length_size = 0;
        char *status_code_buff = NULL;
        size_t status_code_size = 0;
        char *command_buff = NULL;
        size_t command_size = 0;
        char *parameters_buff = NULL;
        size_t parameters_size = 0;
        char *payload_buff = NULL;
        size_t payload_size = 0;

        printf("Client data\n< Request in:\n%s\n", response_buff);
        // Parse the buffer
        load_response(response_buff, &payload_length_buff, &status_code_buff,
                      &command_buff, &parameters_buff, &payload_buff,
                      &payload_length_size, &status_code_size, &command_size,
                      &parameters_size, &payload_size);
        /*fprintf(stderr,"payload_length_buff:%s\n,
        status_code_buff:%s\ncommand_buff:%s\nparameters_buff:%s\npayload_buff:%s\n",
            payload_length_buff,
            status_code_buff,
            command_buff,
            parameters_buff,
            payload_buff
        );*/
        connection = handle_command(command_buff, parameters_buff, payload_buff,
                                    atoi(payload_length_buff), parameters_size,
                                    client_socket, connection);

        memset(response_buff, '\0', DEFAULT_BUFFER_SIZE);
        free(parameters_buff);
    }
    if (n == 0)
    {
        fprintf(stderr, "Client disconnected\n");
        // remove all the room who he was the owner
        if (connection->room != NULL)
        {
            struct room_t *tmp = connection->room;
            while (tmp != NULL)
            {
                if (is_owner_room(tmp, client_socket) == 1)
                {
                    connection->room = remove_room(connection->room, tmp->name);
                    tmp = connection->room;
                }
                else
                    tmp = tmp->next;
            }
        }
        connection = remove_client(connection, client_socket);
        // fflush(stdout);
    }

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
    return connection;
}
