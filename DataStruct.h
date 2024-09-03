#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "cJSON.h"

struct config_json_struct
{
    size_t READ_BUFFER_SIZE;
    size_t CORE_BUFFER_SIZE;
    size_t READ_TIME_INTERTAL;
    size_t HEARTBEAT_TIME_INTERTAL;
    size_t STANDBY_HEARTBEAT_TIME_INTERTAL;
    size_t DATABASE_PERSISTENCE_INTERVAL;
    size_t SERVICE_INSTANCE_TIMEOUT;
    char ADDRESS[16];
    u_int16_t SERVER_PORT;
    u_int16_t MANAGE_PORT;
    char DATABASE_NAME[50];
};

struct connect_struct
{
    size_t READ_BUFFER_SIZE;
    size_t CORE_BUFFER_SIZE;
    size_t READ_TIME_INTERTAL;
    size_t HEARTBEAT_TIME_INTERTAL;
    size_t STANDBY_HEARTBEAT_TIME_INTERTAL;
    size_t DATABASE_PERSISTENCE_INTERVAL;
    char ADDRESS[16];
    u_int16_t PORT;
    cJSON *database;
};

struct receive_data_struct
{
    struct connect_struct connect_data;
    int sockfd;
};