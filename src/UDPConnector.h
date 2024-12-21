#ifndef UDPCONNECTOR_H
#define UDPCONNECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "DataStruct.h"
#include "JsonProcess.h"
#include "cJSON.h"


// 处理服务端和客户端报文的函数
char *processRegisterMessage(cJSON *database, cJSON *data_buffer);
char *processMetaRegisterMessage(cJSON *database, cJSON *data_buffer);
char *processQuery(cJSON *database, cJSON *data_buffer);
void processHeartbeat(cJSON *database, cJSON *data_buffer);


void *UDPconnector(void *args);
void *receive_server(void *arg);



# endif