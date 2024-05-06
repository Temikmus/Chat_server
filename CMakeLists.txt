cmake_minimum_required(VERSION 3.27)
project(Chat_server_project)

set(CMAKE_CXX_STANDARD 17)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
#set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(Chat_server_project main.cpp
        Client.h
        Client.cpp)

add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

target_link_libraries(Chat_server_project wsock32)
