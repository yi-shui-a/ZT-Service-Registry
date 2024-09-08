#ifndef JSONPROCESS_H
#define JSONPROCESS_H

#include <stdio.h>
#include "cJSON.h"

#include "DataStruct.h"

struct config_json_struct config_load(char *filepath);

time_t getReceiveSendTime(cJSON *data);

uint8_t getReceiveType(cJSON *data);

// 添加一个服务实例,传进的消息都不去除报头
//  int addServiceInstance(cJSON *database, cJSON *data);
// 通过注册服务实例
char *addServiceInstanceRegister(cJSON *database, cJSON *data);
// 通过元数据注册服务实例
char *addServiceInstanceMetadata(cJSON *database, cJSON *data);

//获取该服务名的所有服务
char *query(cJSON *database, cJSON *req_data);

void resetHeartbeatTime(cJSON *database, cJSON *data);

char *processResponse(cJSON *response, uint8_t type);

struct config_json_struct config_load(char* filepath);

//如果心跳时间超出间隔，将服务状态置为DOWN
void updateServiceInstanceStatus(cJSON * database,time_t current_time,time_t interval_time);

// 获取一个数字的位数
int getDigit(int i);


cJSON* parseFromStr(char* buffer);

#endif