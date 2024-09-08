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

// #define READ_BUFFER_SIZE 64 * 1024       // 64kB buffer size
// #define CORE_BUFFER_SIZE 4 * 1024 * 1024 // 4MB buffer size
// #define READ_TIME_INTERTAL 1
// #define ADDRESS "192.168.1.100"
// #define PORT 8888

// struct connect_struct config_load(int connection_type);
pthread_mutex_t data_buffer_lock = PTHREAD_MUTEX_INITIALIZER;

void *receive_server(void *arg);

// 处理服务端和客户端报文的函数
char *processRegisterMessage(cJSON *database, cJSON *data_buffer);
char *processMetaRegisterMessage(cJSON *database, cJSON *data_buffer);
char *processQuery(cJSON *database, cJSON *data_buffer);
void processHeartbeat(cJSON *database, cJSON *data_buffer);

/*
connection_type代表通通信的种类：
1：微服务和客户端通信
2：管理端通信
*/
void *UDPconnector(void *args)
{
    struct connect_struct connect_struct = *(struct connect_struct *)args;
    cJSON *database = connect_struct.database;

    int sockfd;
    struct sockaddr_in register_center_addr;

    // 创建UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // 增大操作系统级别的接收缓冲区
    int rcvbuf_size = connect_struct.CORE_BUFFER_SIZE; // 设置为 4MB，或根据需要调整大小
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, sizeof(rcvbuf_size)) < 0)
    {
        perror("Failed to set socket receive buffer size");
    }

    // 初始化客户端地址结构
    memset(&register_center_addr, 0, sizeof(register_center_addr));
    register_center_addr.sin_family = AF_INET;

    // 自动选择ip
    register_center_addr.sin_addr.s_addr = INADDR_ANY;
    // 使用inet_addr将字符串转换为网络字节序的IPv4地址
    // 手动选择ip
    // register_center_addr.sin_addr.s_addr = inet_addr(connect_struct.ADDRESS);
    register_center_addr.sin_port = htons(connect_struct.PORT);
    // 绑定socket
    if (bind(sockfd, (struct sockaddr *)&register_center_addr, sizeof(register_center_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(2);
    }

    printf("Client is ready to receive broadcast messages...\n");
    // printf("%s\n", connect_struct.ADDRESS);
    printf("Client:  %s : %d\n", connect_struct.ADDRESS, connect_struct.PORT);

    // 准备接收消息
    //  // char buffer[1024];
    char *buffer = (char *)malloc(connect_struct.READ_BUFFER_SIZE); // 动态分配内存
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    ssize_t received_len;

    while (1)
    {
        // printf("111\n");
        // 清空 buffer，避免残留数据影响
        memset(buffer, 0, connect_struct.READ_BUFFER_SIZE);
        // 阻塞接收数据
        received_len = recvfrom(sockfd, buffer, connect_struct.READ_BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &addr_len);
        if (received_len < 0)
        {
            perror("Failed to receive message");
            continue;
        }

        buffer[received_len] = '\0'; // 确保字符串以NULL结尾

        printf("received_len: %ld\n", received_len);
        printf("Received message:\n%s\n", buffer);

        // data_buffer = cJSON_Parse("{\"years\":22}");
        pthread_mutex_lock(&data_buffer_lock);
        cJSON *data_buffer = cJSON_Parse(buffer);

        // cJSON *data_buffer = parseFromStr(buffer);

        // printf("%s\n",cJSON_Print(cJSON_Parse(buffer)));
        if (data_buffer == NULL)
        {
            printf("parse fail.\n");
            pthread_mutex_unlock(&data_buffer_lock);
            continue;
            // return;
        }
        // 处理数据
        char response_message[20 * 1024];

        switch (getReceiveType(data_buffer))
        {
        case 1:
            printf("mission: 1\n");
            strcpy(response_message, processRegisterMessage(database, data_buffer));
            break;
        case 3:
            printf("mission: 3\n");
            strcpy(response_message, processMetaRegisterMessage(database, data_buffer));
            break;
        case 5:
            printf("mission: 5\n");
            strcpy(response_message, processQuery(database, data_buffer));
            break;
        case 7:
            printf("mission: 7\n");
            processHeartbeat(database, data_buffer);
            break;
        default:
            printf("Received error message");
            break;
        }

        // 释放消息
        cJSON_Delete(data_buffer);

        pthread_mutex_unlock(&data_buffer_lock);
        // 生成回复消息
        if (sendto(sockfd, response_message, strlen(response_message), 0, (struct sockaddr *)&sender_addr, addr_len) < 0)
        {
            perror("Failed to send response");
        }
        else
        {
            char temp_addr[20] = {0};
            struct in_addr addrptr = sender_addr.sin_addr;
            inet_ntop(AF_INET, &addrptr, temp_addr, sizeof temp_addr);
            printf("Response sent to sender\n");
            printf("IP: %s\n", temp_addr);
            printf("PROT: %d\n", ntohs(sender_addr.sin_port));
            printf("response_message：\n%s\n",response_message);
        }
    }

    free(buffer);
    close(sockfd);
}

// void *receive_server(void *arg)
// {
//     struct receive_data_struct config_arg = *(struct receive_data_struct *)arg;
//     cJSON *database = config_arg.connect_data.database;

//     // 参数为sockfd，套接字描述符
//     int sockfd = config_arg.sockfd;
//     // char buffer[1024];
//     char *buffer = (char *)malloc(config_arg.connect_data.READ_BUFFER_SIZE); // 动态分配内存
//     struct sockaddr_in sender_addr;
//     socklen_t addr_len = sizeof(sender_addr);
//     ssize_t received_len;
//     // cJSON *data_buffer = NULL;

//     while (1)
//     {
//         // printf("111\n");
//         // 清空 buffer，避免残留数据影响
//         memset(buffer, 0, config_arg.connect_data.READ_BUFFER_SIZE);
//         // 阻塞接收数据
//         received_len = recvfrom(sockfd, buffer, config_arg.connect_data.READ_BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &addr_len);
//         if (received_len < 0)
//         {
//             perror("Failed to receive message");
//             continue;
//         }

//         buffer[received_len] = '\0'; // 确保字符串以NULL结尾
//         // char temp_receive_str[received_len+1];
//         // strcpy(temp_receive_str,buffer);
//         // temp_receive_str[received_len] = '\0';

//         printf("received_len: %ld\n", received_len);
//         printf("Received message:\n%s\n", buffer);
//         // printf("%c\n", buffer[received_len]);
//         // printf("1111\n");

//         // data_buffer = cJSON_Parse("{\"years\":22}");

//         cJSON *data_buffer = cJSON_Parse(buffer);
//         // printf("%s\n",cJSON_Print(cJSON_Parse(buffer)));
//         if (data_buffer == NULL)
//         {
//             printf("parse fail.\n");
//             continue;
//             // return;
//         }
//         // 处理数据
//         char response_message[20 * 1024];
//         switch (getReceiveType(data_buffer))
//         {
//         case 1:
//         printf("mission: 1\n");
//             strcpy(response_message, processRegisterMessage(database, data_buffer));
//             break;
//         case 3:
//             printf("mission: 3\n");
//             strcpy(response_message, processMetaRegisterMessage(database, data_buffer));
//             break;
//         case 5:
//         printf("mission: 5\n");
//             strcpy(response_message, processQuery(database, data_buffer));
//             break;
//         case 7:
//         printf("mission: 7\n");
//             processHeartbeat(database, data_buffer);
//             break;
//         default:
//             printf("Received error message");
//             break;
//         }

//         // 释放消息
//         cJSON_Delete(data_buffer);
//         // free(buffer);

//         // 生成回复消息
//         if (sendto(sockfd, response_message, strlen(response_message), 0, (struct sockaddr *)&sender_addr, addr_len) < 0)
//         {
//             perror("Failed to send response");
//         }
//         else
//         {
//             char temp_addr[20]={0};
//             struct in_addr addrptr = sender_addr.sin_addr;
//             inet_ntop(AF_INET, &addrptr, temp_addr, sizeof temp_addr);
//             printf("Response sent to sender\n");
//             printf("IP: %s\n",temp_addr);
//             printf("PROT: %d\n",ntohs(sender_addr.sin_port));
//         }
//     }

//     return NULL;
// }

char *processRegisterMessage(cJSON *database, cJSON *data_buffer)
{
    return addServiceInstanceRegister(database, data_buffer);
}
char *processMetaRegisterMessage(cJSON *database, cJSON *data_buffer)
{
    return addServiceInstanceMetadata(database, data_buffer);
}
char *processQuery(cJSON *database, cJSON *data_buffer)
{
    return query(database, data_buffer);
}
void processHeartbeat(cJSON *database, cJSON *data_buffer)
{
    resetHeartbeatTime(database, data_buffer);
}