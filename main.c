#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "cJSON.h"
#include "UDPConnector.h"
#include "JsonProcess.h"

void getMainLock(struct config_json_struct config_struct); // 获取设备锁
cJSON *initialDatabase(char *filepath);
// void initialService(cJSON *database);
struct connect_struct getConnectStruct(int connection_type, struct config_json_struct config_struct, cJSON *database);
void monitorDatabase(struct config_json_struct config_struct, cJSON *database);
int saveDatabase(cJSON *database, char *filename);

int main()
{

    // 加载配置文件，设置全局变量。
    struct config_json_struct config_struct = config_load("config.json");

    // 获取设备锁
    // 如果能获取，作为主份持续运行;如果不能获取，作为备份，持续心跳，等待成为主份
    getMainLock(config_struct);

    // 定义一个全局的数据库变量
    cJSON *database = initialDatabase(config_struct.DATABASE_NAME);

    /*
    分别启动两个线程的通信
    connection_type代表通通信的种类：
    1：微服务和客户端通信
    2：管理端通信
    */

    // 创建线程来处理通信
    pthread_t thread_server;
    pthread_t thread_manage;

    // 封装配置变量
    struct connect_struct thread_server_config_struct = getConnectStruct(1, config_struct, database);
    struct connect_struct thread_manage_config_struct = getConnectStruct(2, config_struct, database);

    // NULL 表示不使用特定的线程属性，使用默认属性。receive_data 是新线程将要执行的函数。
    // &sockfd 是传递给 receive_data 函数的参数，即文件描述符的地址。在 receive_data 函数内部，您可以通过解引用这个指针来访问实际的文件描述符。

    if (pthread_create(&thread_server, NULL, UDPconnector, &thread_server_config_struct) != 0)
    {
        perror("Failed to create thread_server");
    }

    if (pthread_create(&thread_manage, NULL, UDPconnector, &thread_manage_config_struct) != 0)
    {
        perror("Failed to create thread_manage");
    }

    // 定时进行数据监控
    monitorDatabase(config_struct, database);

    // 线程结束
    pthread_join(thread_server, NULL);
    pthread_join(thread_manage, NULL);
    return 0;
}

cJSON *initialDatabase(char *filepath)
{
    FILE *file = fopen(filepath, "r"); // 尝试以只读模式打开文件

    if (file == NULL)
    {
        cJSON *database = cJSON_CreateObject();
        cJSON_AddArrayToObject(database, "services");
        return database;
    }
    // 查找文件大小
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file); // 重置文件位置指针到文件开头

    // 分配内存以存储文件内容和字符串的终止符
    char *buffer = (char *)malloc(filesize + 1);
    if (buffer == NULL)
    {
        // 内存分配失败
        fclose(file);
        perror("memory allocation failed");
        return NULL;
    }

    // 读取文件内容
    size_t bytesRead = fread(buffer, 1, filesize, file);
    if (bytesRead != filesize)
    {
        // 读取失败或文件在读取过程中被截断
        free(buffer);
        fclose(file);
        perror("Reading failed or the file was truncated during the reading process");
        return NULL;
    }

    // 添加字符串的终止符
    buffer[filesize] = '\0';

    // 关闭文件
    fclose(file);

    // 创建一个cJSON变量
    if (strlen(buffer) < 5)
    {
        cJSON *database = cJSON_CreateObject();
        cJSON_AddArrayToObject(database, "services");
        printf("database loaded!!!");
        // 返回文件内容
        return database;
    }
    cJSON *database = cJSON_Parse(buffer);
    if (cJSON_GetObjectItemCaseSensitive(database, "services") == NULL)
    {
        cJSON_AddArrayToObject(database, "services");
    }

    printf("database loaded!!!");

    // 返回文件内容
    return database;
}

struct connect_struct getConnectStruct(int connection_type, struct config_json_struct config_struct, cJSON *database)
{
    struct connect_struct new_struct;
    new_struct.READ_BUFFER_SIZE = config_struct.READ_BUFFER_SIZE;
    new_struct.CORE_BUFFER_SIZE = config_struct.CORE_BUFFER_SIZE;
    new_struct.READ_TIME_INTERTAL = config_struct.READ_TIME_INTERTAL;
    new_struct.HEARTBEAT_TIME_INTERTAL = config_struct.HEARTBEAT_TIME_INTERTAL;
    new_struct.STANDBY_HEARTBEAT_TIME_INTERTAL = config_struct.STANDBY_HEARTBEAT_TIME_INTERTAL;
    new_struct.DATABASE_PERSISTENCE_INTERVAL = config_struct.DATABASE_PERSISTENCE_INTERVAL;
    strcpy(new_struct.ADDRESS, config_struct.ADDRESS);
    // 将database也传入线程
    new_struct.database = database;
    // 判断端口类型
    if (connection_type == 1)
    {
        new_struct.PORT = config_struct.SERVER_PORT;
    }
    else if (connection_type == 2)
    {
        new_struct.PORT = config_struct.MANAGE_PORT;
    }

    return new_struct;
}

// 求两个数的最大公因数（GCD），使用迭代方法
int gcd(int a, int b)
{
    while (b != 0)
    {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// 求多个数的最大公因数
int gcdMultiple(int nums[], int size)
{
    if (size == 0)
        return 0;         // 如果数组为空，则没有最大公因数
    int result = nums[0]; // 从第一个数开始
    for (int i = 1; i < size; i++)
    {
        result = gcd(result, nums[i]); // 逐个与后面的数求GCD
    }
    printf("gcdMultiple:%d\n", result);
    return result;
}

void monitorDatabase(struct config_json_struct config_struct, cJSON *database)
{
    int temp_array[2] = {config_struct.DATABASE_PERSISTENCE_INTERVAL, config_struct.SERVICE_INSTANCE_TIMEOUT};
    int check_interval = gcdMultiple(temp_array, 2);
    time_t start_time;
    time(&start_time);
    size_t database_persistence_interval_count = 0;
    size_t service_instance_time_count = 0;
    // printf("finished save databse\n");
    while (1)
    {

        sleep(check_interval);
        time_t current_time;
        time(&current_time);
        if ((current_time - start_time) / config_struct.DATABASE_PERSISTENCE_INTERVAL >= database_persistence_interval_count)
        {
            printf("finished save databse\n");
            // 保存文件
            saveDatabase(database, config_struct.DATABASE_NAME);
            // 计数器+1
            database_persistence_interval_count += 1;
        }
        if ((current_time - start_time) >= 5)
        {

            // 遍历所有服务实例，更新其状态
            updateServiceInstanceStatus(database, current_time, config_struct.SERVICE_INSTANCE_TIMEOUT);
            // 计数器+1
            service_instance_time_count += 1;
        }
    }
}

int saveDatabase(cJSON *database, char *filename)
{
    char *database_str = cJSON_Print(database);
    printf("save database:\n%s\n", database_str);

    // 将字符串写入文件
    FILE *file = fopen(filename, "w");
    if (database_str == NULL)
    {
        perror("database is null!!!\n");
        if (file != NULL)
            fclose(file);

        return -1; // 或其他错误码
    }
    fprintf(file, "%s", database_str);
    fclose(file);

    // 释放字符串分配的内存
    free(database_str);
    return 1;
}

// 尝试获取文件锁
int try_lock_file(const char *lockfile)
{
    int fd = open(lockfile, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
    {
        perror("open lockfile error");
        return -1;
    }

    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_WRLCK; // 写锁定
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0; // 锁定整个文件

    // 尝试获取文件锁（非阻塞）
    if (fcntl(fd, F_SETLK, &fl) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return 0; // 锁已经被其他进程持有
        }
        else
        {
            perror("fcntl error");
            close(fd);
            return -1;
        }
    }

    return fd; // 锁获取成功
}

// 释放文件锁
void unlock_file(int fd)
{
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    fl.l_type = F_UNLCK; // 释放锁
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0; // 解锁整个文件

    fcntl(fd, F_SETLK, &fl);
    close(fd);
}

void getMainLock(struct config_json_struct config_struct)
{
    char *lockfile = config_struct.LOCKFILE;
    time_t STANDBY_HEARTBEAT_TIME_INTERTAL = config_struct.STANDBY_HEARTBEAT_TIME_INTERTAL;
    // 获取文件锁句柄
    int fd;
    while (1)
    {
        fd = try_lock_file(lockfile);
        if (fd > 0)
        {
            printf("[INFO] %s Became PRIMARY.\n", config_struct.ADDRESS);
            break;
        }
        else if (fd == 0)
        {
            printf("[INFO] %s Became BACKUP, monitoring MAIN.\n", config_struct.ADDRESS);
        }
        else
        {
            printf("[ERROR] %s Failed to check lock.\n", config_struct.ADDRESS);
        }
        // 检查主服务器是否存活（超时机制）
        sleep(STANDBY_HEARTBEAT_TIME_INTERTAL);
        fd = try_lock_file(lockfile);
        if (fd > 0)
        {
            printf("[INFO] PRIMARY server timeout, %s becoming PRIMARY.\n", config_struct.ADDRESS);
            break;
        }
    }
}
