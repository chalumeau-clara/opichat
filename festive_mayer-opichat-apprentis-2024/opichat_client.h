#ifndef BASIC_CLIENT_H_
#define BASIC_CLIENT_H_
#define _POSIX_C_SOURCE 2001112L

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEFAULT_BUFFER_SIZE 2048
struct addrinfo;
/**
 * \brief Iterate over the struct addrinfo elements to connect to the server
 *
 * \param addrinfo: struct addrinfo elements
 *
 * \return The created socket or exit with 1 if there is an error
 *
 * Try to create and connect a socket with every addrinfo element until it
 * succeeds
 *
 */
int create_and_connect(struct addrinfo *addrinfo);

/**
 * \brief Initialize the addrinfo struct and call create_and_connect()
 *
 * \param ip: IP address of the server
 * \param port: Port of the server
 *
 * \return The created socket
 */
int prepare_socket(const char *ip, const char *port);

/**
 * \brief Handle communication with the server
 *
 * \param server_socket: server socket
 *
 * Read user message from STDIN and send it to the server
 *
 * Step 2: This function sends small messages to the server
 *
 * Step 3: In addition to step 2 behavior, this function receives the server
 * response on small messages
 *
 * Step 4: The function must now handle long messages
 */
void *communicate(void *server_socket_ptr);

int is_valid_commands(char *buffer);

void managing_threads(int server_socket);

int is_alpha_numerical_character(char c);

int is_valid_send_parameters(char *rest);

int has_parameters_recv(char *buffer);

int has_parameters_send(char *buffer);

void cp_str(char *str, size_t *i, char *sstr, size_t si);

void get_request(char *buff_request, char *buff_command_tmp,
                 char *buff_parameters_tmp, char *buff_payload_tmp,
                 size_t size_payload, size_t size_parameters,
                 size_t *size_request);

char *get_sender(char *parameters);

char *get_room(char *parameters);

void load_response(char *response_buffer, char **payload_length_buff,
                   char **status_code_buff, char **command_buff,
                   char **parameters_buff, char **payload_buff,
                   size_t *payload_length_size, size_t *status_code_size,
                   size_t *command_size, size_t *parameters_size,
                   size_t *payload_size);
#endif /* BASIC_CLIENT_H_ */
