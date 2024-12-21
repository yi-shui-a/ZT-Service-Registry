#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

// 从文件读取JSON数据并转换为字符串
char *read_json_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("无法打开文件: %s\n", filename);
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 动态分配内存以读取整个文件内容
    char *data = (char *)malloc(length + 1);
    if (!data) {
        printf("内存分配失败\n");
        fclose(file);
        return NULL;
    }

    // 读取文件内容到缓冲区
    fread(data, 1, length, file);
    data[length] = '\0'; // 确保字符串以空字符结尾

    fclose(file);
    return data;
}

int main() {
    // 读取JSON文件
    const char *filename = "metadata.json"; // 请确保文件名正确
    char *json_data = read_json_file(filename);

    if (json_data == NULL) {
        printf("读取文件失败\n");
        return 1;
    }

    printf("读取的JSON数据:\n%s\n", json_data);

    // 解析JSON字符串
    cJSON *json = cJSON_Parse(json_data);
    if (json == NULL) {
        printf("JSON解析失败\n");
        free(json_data);
        return 1;
    }

    printf("JSON解析成功\n");

    // 在这里处理cJSON对象
    // 例如获取某个键的值
    cJSON *name = cJSON_GetObjectItem(json, "name");
    if (cJSON_IsString(name) && (name->valuestring != NULL)) {
        printf("name: %s\n", name->valuestring);
    }

    // 清理资源
    cJSON_Delete(json);
    free(json_data);
    return 0;
}
