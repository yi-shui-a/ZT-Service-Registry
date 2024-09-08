#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
  
#define BUFFER_SIZE 1024 *8 
#define PORT 8888
#define BROADCAST_ADDR "255.255.255.255"  
struct sockaddr_in broadcastAddr;  
  
// 函数声明  
void broadcast_file(int sock, const char *filename, const char *broadcast_ip, int port);  
  
int main(int argc, char *argv[]) {  
    int sock;  
    
  
    if (argc != 2) {  
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);  
        return EXIT_FAILURE;  
    }  
  
    const char *filename = argv[1];  
  
    // 创建UDP socket  
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {  
        perror("socket creation failed");  
        exit(EXIT_FAILURE);  
    }  
  
    // 设置socket选项以允许广播  
    int opt = 1;  
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0) {  
        perror("setsockopt(SO_BROADCAST) failed");  
        exit(EXIT_FAILURE);  
    }  
  
    // 初始化广播地址结构  
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));  
    broadcastAddr.sin_family = AF_INET;  
    broadcastAddr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR);  
    broadcastAddr.sin_port = htons(PORT);  
  
    while (1) { // 无限循环，每30秒发送一次  
        broadcast_file(sock, filename, BROADCAST_ADDR, PORT);  
        sleep(5);  
    }  
  
    close(sock);  
    return 0;  
}  
  
// 实现：读取文件并通过UDP广播发送  
void broadcast_file(int sock, const char *filename, const char *broadcast_ip, int port) {  
    FILE *file = fopen(filename, "r");  
    if (file == NULL) {  
        perror("Failed to open file");  
        return;  
    }  
  
    char buffer[BUFFER_SIZE];  
    ssize_t numRead;  
  
    while ((numRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {  
        printf("%s\n",buffer);
        if (sendto(sock, buffer, numRead, 0,  
                   (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) != numRead) {  
            perror("sendto() sent a different number of bytes than expected");  
        }  
    }  
  
    fclose(file);  
}  
  
// 注意：broadcastAddr在broadcast_file函数外部定义，因此需要在broadcast_file中作为全局变量或参数传递