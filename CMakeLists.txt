cmake_minimum_required(VERSION 3.10)
project(select-epoll-chatroom)

add_executable(server server.cpp)
add_executable(client client.cpp)