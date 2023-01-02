#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include "../opichat_client.h"

Test(BasicInvalidCommand, is_valid_command)
{
    int expected = 0;
    int got = is_valid_commands("PINUG");
    cr_assert_eq(got, expected, "Expected: %d. Got: %d", expected, got);
}

Test(BasicValidCommand, is_valid_command)
{
    int expected = 1;
    int got = is_valid_commands("PING\n");
    cr_assert_eq(got, expected, "Expected: %d. Got: %d", expected, got);
}

Test(BasicValidAlphaNum, is_alpha_numerical_character)
{
    int expected = 1;
    int got = is_alpha_numerical_character('c');
    cr_assert_eq(got, expected, "Expected: %d. Got: %d", expected, got);
}

Test(BasicInvalidAlphaNum, is_alpha_numerical_character)
{
    int expected = 0;
    int got = is_alpha_numerical_character('#');
    cr_assert_eq(got, expected, "Expected: %d. Got: %d", expected, got);
}

Test(BasicValidSendParams, is_valid_send_parameters)
{
    int expected = 1;
    int got = is_valid_send_parameters("User=q");
    cr_assert_eq(got, expected, "Expected: %d. Got: %d", expected, got);
}

Test(BasicInvalidSendParams, is_valid_send_parameters)
{
    int expected = 0;
    int got = is_valid_send_parameters("User");
    cr_assert_eq(got, expected, "Expected: %d. Got: %d", expected, got);
}

Test(BasicValidHasSendParams, has_parameters_send)
{
    int expected = 1;
    int got = has_parameters_send("SEND-DM");
    cr_assert_eq(got, expected, "Expected: %d. Got: %d", expected, got);
}

Test(BasicInvalidHasSendParams, has_parameters_send)
{
    int expected = 0;
    int got = has_parameters_send("PING");
    cr_assert_eq(got, expected, "Expected: %d. Got: %d", expected, got);
}

Test(BasicValidHasRecvParams, has_parameters_recv)
{
    int expected = 1;
    int got = has_parameters_recv("SEND-ROOM\n");
    cr_assert_eq(got, expected, "Expected: %d. Got: %d", expected, got);
}

Test(BasicValidCpStr, cp_str)
{
    char *expected = "PING\0";
    char str[5];
    size_t i = 0;
    cp_str(str, &i, expected, 5);
    cr_assert_str_eq(str, expected, "Expected: %s. Got: %s", expected, str);
}

Test(BasicNotCompleteCpStr, cp_str)
{
    char *expected = "PING";
    char str[5];
    size_t i = 0;
    cp_str(str, &i, "PING\0", 4);
    cr_assert_str_neq(str, expected, "Expected: %s. Got: %s", expected, str);
}

// Basic
Test(PINGGetResquest, get_request)
{
    char *expected = "0\n0\nPING\n";
    char buff_request[10000];
    char *buff_command_tmp = "PING";
    char *buff_parameters_tmp = "";
    char *buff_payload_tmp = "";
    size_t size_payload = 0;
    size_t size_parameters = 0;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

Test(LOGINGetResquest, get_request)
{
    char *expected = "3\n0\nLOGIN\n\nacu";
    char buff_request[10000];
    char *buff_command_tmp = "LOGIN\n";
    char *buff_parameters_tmp = "";
    char *buff_payload_tmp = "acu\n";
    size_t size_payload = 3;
    size_t size_parameters = 0;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

Test(LISTUSERSGetResquest, get_request)
{
    char *expected = "0\n0\nLIST-USERS\n";
    char buff_request[10000];
    char *buff_command_tmp = "LIST-USERS";
    char *buff_parameters_tmp = "";
    char *buff_payload_tmp = "";
    size_t size_payload = 0;
    size_t size_parameters = 0;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

// With payload & parameters
Test(SENDDMGetResquest, get_request)
{
    char *expected = "4\n0\nSEND-DM\nUser=acu\n\n2022";
    char buff_request[1000];
    char *buff_command_tmp = "SEND-DM\n";
    char *buff_parameters_tmp = "User=acu\n";
    char *buff_payload_tmp = "2022\n";
    size_t size_payload = 4;
    size_t size_parameters = 9;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

Test(BROADCASTGetResquest, get_request)
{
    char *expected = "4\n0\nBROADCAST\n\n2022";
    char buff_request[10000];
    char *buff_command_tmp = "BROADCAST\n";
    char *buff_parameters_tmp = "";
    char *buff_payload_tmp = "2022\n";
    size_t size_payload = 4;
    size_t size_parameters = 0;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

// With payload
Test(CREATEROOMGetResquest, get_request)
{
    char *expected = "8\n0\nCREATE-ROOM\n\nFlagRoom";
    char buff_request[1000];
    char *buff_command_tmp = "CREATE-ROOM\n";
    char *buff_parameters_tmp = "";
    char *buff_payload_tmp = "FlagRoom\n";
    size_t size_payload = 8;
    size_t size_parameters = 0;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

Test(LISTROOMSGetResquest, get_request)
{
    char *expected = "0\n0\nLIST-ROOMS\n";
    char buff_request[10000];
    char *buff_command_tmp = "LIST-ROOMS";
    char *buff_parameters_tmp = "";
    char *buff_payload_tmp = "";
    size_t size_payload = 0;
    size_t size_parameters = 0;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

Test(JOINROOMGetResquest, get_request)
{
    char *expected = "8\n0\nJOIN-ROOM\n\nFlagRoom";
    char buff_request[10000];
    char *buff_command_tmp = "JOIN-ROOM\n";
    char *buff_parameters_tmp = "";
    char *buff_payload_tmp = "FlagRoom\n";
    size_t size_payload = 8;
    size_t size_parameters = 0;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

Test(LEAVEROOMGetResquest, get_request)
{
    char *expected = "8\n0\nLEAVE-ROOM\n\nFlagRoom";
    char buff_request[10000];
    char *buff_command_tmp = "LEAVE-ROOM\n";
    char *buff_parameters_tmp = "";
    char *buff_payload_tmp = "FlagRoom\n";
    size_t size_payload = 8;
    size_t size_parameters = 0;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

Test(SENDROOMGetResquest, get_request)
{
    char *expected = "4\n0\nSEND-ROOM\nRoom=FlagRoom\n\n2022";
    char buff_request[10000];
    char *buff_command_tmp = "SEND-ROOM\n";
    char *buff_parameters_tmp = "Room=FlagRoom\n";
    char *buff_payload_tmp = "2022\n";
    size_t size_payload = 4;
    size_t size_parameters = 14;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

Test(DELETEROOMGetResquest, get_request)
{
    char *expected = "8\n0\nDELETE-ROOM\n\nFlagRoom";
    char buff_request[10000];
    char *buff_command_tmp = "DELETE-ROOM\n";
    char *buff_parameters_tmp = "";
    char *buff_payload_tmp = "FlagRoom\n";
    size_t size_payload = 8;
    size_t size_parameters = 0;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}

Test(PROFILEGetResquest, get_request)
{
    char *expected = "0\n0\nPROFILE\n";
    char buff_request[10000];
    char *buff_command_tmp = "PROFILE";
    char *buff_parameters_tmp = "";
    char *buff_payload_tmp = "";
    size_t size_payload = 0;
    size_t size_parameters = 0;
    size_t size_request = 0;
    get_request(buff_request, buff_command_tmp, buff_parameters_tmp,
                buff_payload_tmp, size_payload, size_parameters, &size_request);
    cr_assert_str_eq(buff_request, expected, "Expected: %s. Got: %s", expected,
                     buff_request);
}
Test(BasicGetSender, get_sender)
{
    char *expected = "clara";
    char params[] = "User=acu\nFrom=clara\n";
    char *sender = get_sender(params);
    cr_assert_str_eq(sender, expected, "Expected: %s. Got: %s", expected,
                     sender);
}

Test(BasicGetRoom, get_room)
{
    char *expected = "epita";
    char params[] = "Room=epita\n";
    char *room = get_room(params);
    cr_assert_str_eq(room, expected, "Expected: %s. Got: %s", expected, room);
}

Test(SENDROOMoadResponse, load_response)
{
    char response_buff[10000] = "0\n1\nSEND-ROOM\nROOM=FlagRoom\n\n";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "0", "Expected: %s. Got: %s", "0",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "SEND-ROOM", "Expected: %s. Got: %s",
                     "SEND-ROOM", command_buff);
}

Test(DELETEROOMLoadResponse, load_response)
{
    char response_buff[10000] = "13\n1\nDELETE-ROOM\n\nRoom deleted";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "13", "Expected: %s. Got: %s", "13",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "DELETE-ROOM", "Expected: %s. Got: %s",
                     "DELETE-ROOM", command_buff);
}

Test(LEAVEROOMLoadResponse, load_response)
{
    char response_buff[10000] = "10\n1\nLEAVE-ROOM\n\nRoom left";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "10", "Expected: %s. Got: %s", "10",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "LEAVE-ROOM", "Expected: %s. Got: %s",
                     "LEAVE-ROOM", command_buff);
}

Test(JOINROOMLoadResponse, load_response)
{
    char response_buff[10000] = "12\n1\nJOIN-ROOM\n\nRoom joined";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "12", "Expected: %s. Got: %s", "12",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "JOIN-ROOM", "Expected: %s. Got: %s",
                     "JOIN-ROOM", command_buff);
}

Test(LISTROOMSLoadResponse, load_response)
{
    char response_buff[10000] =
        "24\n1\nLIST-ROOMS\n\nCISCO\nLABSR\nMIDLAB\nSM14";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "24", "Expected: %s. Got: %s", "24",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "LIST-ROOMS", "Expected: %s. Got: %s",
                     "LIST-ROOMS", command_buff);
}

Test(CREATEROOMLoadResponse, load_response)
{
    char response_buff[10000] = "13\n1\nCREATE-ROOM\n\nRoom created";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "13", "Expected: %s. Got: %s", "13",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "CREATE-ROOM", "Expected: %s. Got: %s",
                     "CREATE-ROOM", command_buff);
}

Test(BROADCASTLoadResponse, load_response)
{
    char response_buff[10000] = "0\n1\nBROADCAST\n\n";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "0", "Expected: %s. Got: %s", "0",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "BROADCAST", "Expected: %s. Got: %s",
                     "BROADCAST", command_buff);
}

Test(SENDDMLoadResponse, load_response)
{
    char response_buff[10000] = "0\n1\nSEND-DM\nUser=acu\n\n";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "0", "Expected: %s. Got: %s", "0",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "SEND-DM", "Expected: %s. Got: %s",
                     "SEND-DM", command_buff);
}

Test(LISTUSERSLoadResponse, load_response)
{
    char response_buff[10000] = "15\n1\nLIST-USERS\n\nacu\nHappy\nING1";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "15", "Expected: %s. Got: %s", "15",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "LIST-USERS", "Expected: %s. Got: %s",
                     "LIST-USERS", command_buff);
}

Test(LOGINLoadResponse, load_response)
{
    char response_buff[10000] = "10\n1\nLOGIN\n\nLogged in";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "10", "Expected: %s. Got: %s", "10",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "LOGIN", "Expected: %s. Got: %s", "LOGIN",
                     command_buff);
}

Test(PINGLoadResponse, load_response)
{
    char response_buff[10000] = "5\n1\nPING\n\nPONG";
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
    load_response(response_buff, &payload_length_buff, &status_code_buff,
                  &command_buff, &parameters_buff, &payload_buff,
                  &payload_length_size, &status_code_size, &command_size,
                  &parameters_size, &payload_size);
    cr_assert_str_eq(payload_length_buff, "5", "Expected: %s. Got: %s", "5",
                     payload_length_buff);
    free(parameters_buff);
    free(payload_buff);
    cr_assert_str_eq(status_code_buff, "1", "Expected: %s. Got: %s", "1",
                     status_code_buff);
    cr_assert_str_eq(command_buff, "PING", "Expected: %s. Got: %s", "PING",
                     command_buff);
}
