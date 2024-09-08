gcc cJSON.c DataStruct.c JsonProcess.c  UDPConnector.c -pthread main.c -o registry

gcc z_test.c -o z_test

gcc z_test_json.c cJSON.c -o z_test_json