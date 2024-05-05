cmake_minimum_required(VERSION 3.27)
project(Chat_server_project)

set(CMAKE_CXX_STANDARD 17)



add_executable(Chat_server_project main.cpp)

add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)

target_link_libraries(Chat_server_project wsock32)
