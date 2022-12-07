cmake_minimum_required(VERSION 3.23)
project(API1_0 C)

set(CMAKE_C_STANDARD 99)

add_executable(API1_0 main.c SpeedPlan.c SpeedPlan.h define.h log.c log.h socket.c socket.h init.c init.h WebServer.c WebServer.h WebCom.c WebCom.h cJSON.c cJSON.h can.c can.h)
target_link_libraries(API1_0 m pthread)
