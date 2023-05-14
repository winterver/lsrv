cmake_minimum_required(VERSION 3.15)
project(lsrv)

include_directories(lib)
link_directories(lib)

add_executable(lsrv src/server.cpp lib/wepoll.c)
add_executable(clnt src/client.cpp)

target_link_libraries(lsrv PRIVATE ws2_32)
target_link_libraries(clnt PRIVATE ws2_32)
