#include "opichat_client.h"

#include <netdb.h>
#include <pthread.h>
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

// Return 1 if valid command, 0 otherwise
int is_valid_commands(char *buffer)
{
    if ((strcmp(buffer, "PING\n") == 0) || (strcmp(buffer, "LOGIN\n") == 0)
        || (strcmp(buffer, "LIST-USERS\n") == 0)
        || (strcmp(buffer, "SEND-DM\n") == 0)
        || (strcmp(buffer, "BROADCAST\n") == 0)
        || (strcmp(buffer, "CREATE-ROOM\n") == 0)
        || (strcmp(buffer, "LIST-ROOMS\n") == 0)
        || (strcmp(buffer, "JOIN-ROOM\n") == 0)
        || (strcmp(buffer, "LEAVE-ROOM\n") == 0)
        || (strcmp(buffer, "SEND-ROOM\n") == 0)
        || (strcmp(buffer, "DELETE-ROOM\n") == 0)
        || (strcmp(buffer, "PROFILE\n") == 0))
    {
        return 1;
    }
    return 0;
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

// Return 1 if command has parameters to send, 0 otherwise
int has_parameters_send(char *buffer)
{
    if ((strstr(buffer, "SEND-DM") != NULL)
        || (strstr(buffer, "SEND-ROOM") != NULL))
    {
        return 1;
    }
    return 0;
}

// Return 1 if command has parameters to receive, 0 otherwise
int has_parameters_recv(char *buffer)
{
    if ((strstr(buffer, "BROADCAST") != NULL)
        || (strcmp(buffer, "SEND-ROOM\n") == 0)
        || (strcmp(buffer, "SEND-ROOM") == 0))
    {
        return 1;
    }
    return 0;
}

// Load the sstr of si size un str at i index
void cp_str(char *str, size_t *i, char *sstr, size_t si)
{
    for (size_t m = 0; m < si; m++)
    {
        str[*i] = sstr[m];
        *i = *i + 1;
    }
}

// Get parameters of stdin
int get_parameters(char *buff_tot, size_t *size_parameters)
{
    ssize_t stdin_size = 0;
    size_t buff_size = 0;
    char *buff = NULL;
    while ((stdin_size = getline(&buff, &buff_size, stdin)) > 1)
    {
        if (strstr(buff, "=") != NULL
            && (buff[0] != '=' && buff[stdin_size - 2] != '='))
        {
            cp_str(buff_tot, size_parameters, buff, stdin_size);
        }
        else
        {
            fprintf(stderr, "Invalid parameter\n");
            free(buff);
            return -1;
        }
    }
    free(buff);
    return 0;
}

// Get payload of stdin
int get_payload(char *buff_tot, char *buff_command, size_t *size_payload)
{
    fprintf(stdout, "Payload:\n");
    fflush(stdin);
    char ch;
    while (ch != '\n' && ch != EOF)
    {
        ch = getchar();
        buff_tot[(*size_payload)++] = ch;
    }
    if (strcmp(buff_tot, "/quit\n") == 0)
    {
        return -1;
    }

    // I do not understand why it doing on overflow with the moulinette and not
    // when I test it myself
    /*if (strstr(buff_command, "SEND-DM") != NULL
        || strstr(buff_command, "BROADCAST") != NULL
        || strstr(buff_command, "SEND-ROOM") != NULL)
    {
        return 0;
    }*/
    (void)buff_command;
    return 1;
}

// Concatenate all the request into one buffer
void get_request(char *buff_request, char *buff_command, char *buff_parameters,
                 char *buff_payload, size_t size_payload,
                 size_t size_parameters, size_t *size_request)
{
    // Size payload
    char str_payload[10000];
    if (strlen(buff_payload) != 0)
        size_payload = strlen(buff_payload) - 1;
    sprintf(str_payload, "%ld", size_payload);
    cp_str(buff_request, size_request, str_payload, strlen(str_payload));
    buff_request[*size_request] = '\n';

    // Status Code for request
    *size_request = *size_request + 1;
    buff_request[*size_request] = '0';
    *size_request = *size_request + 1;
    buff_request[*size_request] = '\n';

    // Command
    *size_request = *size_request + 1;
    size_t size_command = strlen(buff_command);
    cp_str(buff_request, size_request, buff_command, size_command);

    // Parameters
    if (has_parameters_send(buff_command) == 1)
    {
        cp_str(buff_request, size_request, buff_parameters, size_parameters);
    }

    buff_request[*size_request] = '\n';
    *size_request = *size_request + 1;

    // Payload
    cp_str(buff_request, size_request, buff_payload, size_payload);
    buff_request[*size_request] = '\0';
}

// manage all the request
void *communicate(void *server_socket_ptr)
{
    int server_socket = *(int *)server_socket_ptr;
    socket_id = server_socket;

    // Client leaving
    signal(SIGINT, sigint_leave_handler);

    while (1)
    {
        // Get command
        fprintf(stdout, "Command:\n");
        char *buff_command = NULL;
        size_t buff_size = 0;
        ssize_t stdin_size = -1;
        stdin_size = getline(&buff_command, &buff_size, stdin);
        if (stdin_size == -1)
        {
            // fprintf(stderr, "Can not read from client socket stdin");
            exit(1);
        }
        if (stdin_size > 0 && buff_command[stdin_size - 1] != '\n')
        {
            close(server_socket);
            exit(0);
        }
        if (stdin_size == 0 && buff_command[0] != '\n')
        {
            close(server_socket);
            exit(0);
        }

        // Verify if the command is an invalid command
        if (is_valid_commands(buff_command) == 0)
        {
            fprintf(stderr, "Invalid command\n");
            free(buff_command);
            continue;
        }

        // get parameters if it has
        char buff_parameters[10000];
        size_t size_parameters = 0;

        // If it has parameter, get it
        if (has_parameters_send(buff_command) == 1)
        {
            fprintf(stdout, "Parameters:\n");
            while (get_parameters(buff_parameters, &size_parameters) == -1)
                ;
        }

        // Get payload
        char buff_payload[10000];
        size_t size_payload = 0;
        char buff_request[10000];

        int err_code;
        while (
            (err_code = get_payload(buff_payload, buff_command, &size_payload))
            != -1)
        {
            // Get request
            size_t size_request = 0;
            get_request(buff_request, buff_command, buff_parameters,
                        buff_payload, size_payload, size_parameters,
                        &size_request);

            // Send request to the server
            ssize_t send_size = send(server_socket, buff_request, size_request,
                                     MSG_DONTWAIT | MSG_NOSIGNAL);
            if (send_size == -1)
            {
                close(server_socket);
                // fprintf(stderr, "Something wents wrong when send to server");
                exit(1);
            }
            size_payload = 0;
            memset(buff_payload, 0, 10000);
            memset(buff_request, 0, 10000);
            if (err_code == 1)
                break;
        }
        free(buff_command);
    }

    // Close and exit function
    close(server_socket);
    pthread_exit(NULL);
}

// Parse server response
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
    if ((has_parameters_recv(*command_buff) == 1
         || is_valid_send_parameters(rest) == 1)
        && atoi(*status_code_buff) != 1)
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
        (*parameters_buff)[parameters_index] = '\0';
        *parameters_size = strlen(*parameters_buff);
    }
    // Get payloads
    *payload_size = atoi(*payload_length_buff);
    if (*payload_size != 0)
    {
        *payload_buff = malloc(10000);
        size_t payload_index = 0;
        rest++;
        cp_str(*payload_buff, &payload_index, rest, *payload_size);
        (*payload_buff)[*payload_size] = '\0';
    }
    /*
        printf("status code = %s\n", *status_code_buff);
        printf("payload size = %s\n", payload_length_buff);
        printf("status code = %s\n", status_code_buff);
        printf("command = %s\n", command_buff);
        printf("parameters = %s\n", parameters_buff);
    */
    //(*payload_buff)[*payload_size] = '\0';
}

// get the sender in the parameter buffer
char *get_sender(char *parameters)
{
    char *rest = NULL;
    char *token = NULL;
    token = strtok_r(parameters, "\n", &rest);
    while (strstr(token, "From") == NULL)
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

// Manage server response
void *listen_server(void *server_socket_ptr)
{
    int server_socket = *(int *)server_socket_ptr;
    while (1)
    {
        char response_buff[10000];
        size_t response_buff_size = 10000;

        // receive response
        ssize_t server_response_size = -1;
        while ((server_response_size =
                    recv(server_socket, response_buff, response_buff_size, 0))
               < 0)
            ;
        if (server_response_size == -1)
        {
            // fprintf(stderr, "Error while receiving server response");
            continue;
        }
        response_buff[server_response_size] = '\0';

        // Init all the buffer
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
        char tmp_param[10000];
        char tmp_param1[10000];

        // Parse the buffer
        load_response(response_buff, &payload_length_buff, &status_code_buff,
                      &command_buff, &parameters_buff, &payload_buff,
                      &payload_length_size, &status_code_size, &command_size,
                      &parameters_size, &payload_size);

        int status_code = atoi(status_code_buff);

        // If receive notification
        if (status_code == 2
            && (strstr(command_buff, "SEND-DM") != NULL
                || strstr(command_buff, "BROADCAST") != NULL
                || strstr(command_buff, "SEND-ROOM") != NULL))
        {
            // copy the parameter buffer
            size_t len = 0;
            cp_str(tmp_param1, &len, parameters_buff, parameters_size);
            len = 0;
            cp_str(tmp_param, &len, parameters_buff, parameters_size);

            // get the sender
            char *sender = get_sender(tmp_param1);
            if (payload_size != 0)
            {
                if (strstr(command_buff, "BROADCAST") != NULL)
                    fprintf(stdout, "From %s: %s\n", sender, payload_buff);
                else if (strstr(command_buff, "SEND-ROOM") != NULL)
                    fprintf(stdout, "From %s@%s: %s\n", sender,
                            get_room(tmp_param), payload_buff);
                else
                    fprintf(stdout, "From %s: %s\n", sender, payload_buff);
            }
            memset(tmp_param, '\0', 10000);
            memset(tmp_param1, '\0', 10000);
        }

        // Receive normal
        else if (payload_size != 0 && status_code == 1)
            fprintf(stdout, "< %s", payload_buff);

        // Receive error
        else if (payload_size != 0 && status_code == 3)
            fprintf(stderr, "! %s", payload_buff);
        free(parameters_buff);
        free(payload_buff);
    }
    // Close and exit socket
    close(server_socket);
    pthread_exit(NULL);
}

// Managing 2 thread
void managing_threads(int server_socket)
{
    pthread_t send_thread;
    pthread_t rcv_thread;

    // Create thread
    int err_code =
        pthread_create(&send_thread, NULL, communicate, &server_socket);
    if (err_code != 0)
    {
        fprintf(stderr, "thread creating failed");
        exit(1);
    }
    err_code = pthread_create(&rcv_thread, NULL, listen_server, &server_socket);
    if (err_code != 0)
    {
        fprintf(stderr, "thread creating failed");
        exit(1);
    }

    // Close thread
    err_code = pthread_join(send_thread, NULL);
    if (err_code != 0)
    {
        fprintf(stderr, "thread join failed");
        exit(1);
    }
    err_code = pthread_join(rcv_thread, NULL);
    if (err_code != 0)
    {
        fprintf(stderr, "thread join failed");
        exit(1);
    }
}
