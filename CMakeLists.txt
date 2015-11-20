cmake_minimum_required(VERSION 2.6)
project(tcp_thserver)

add_executable(tcp_thserver main.cpp)

install(TARGETS tcp_thserver RUNTIME DESTINATION bin)
