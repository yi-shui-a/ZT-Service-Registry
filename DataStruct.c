#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "cJSON.h"



struct connect_struct
{
    size_t READ_BUFFER_SIZE;
    size_t CORE_BUFFER_SIZE;
    size_t READ_TIME_INTERTAL;
    size_t HEARTBEAT_TIME_INTERTAL;
    char ADDRESS[16];
    u_int16_t PORT;
};

struct receive_data_struct
{
    struct connect_struct connect_data;
    int sockfd;
    cJSON* database;
};

struct connect_database_struct
{
    int connection_type;
    cJSON* database;
};
