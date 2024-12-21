DIRECTORY="bin"
 
if [ ! -d "$DIRECTORY" ]; then
  mkdir -p "$DIRECTORY"
fi

cd src/

gcc cJSON.c DataStruct.c JsonProcess.c  UDPConnector.c -pthread main.c -o ../bin/registry

gcc z_test.c -o ../bin/z_test

# gcc z_test_json.c cJSON.c -o z_test_json