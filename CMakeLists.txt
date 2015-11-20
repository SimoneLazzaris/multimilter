cmake_minimum_required(VERSION 2.6)
project(tcp_thserver)

add_executable(tcp_thserver main.cpp thpool.cpp)
set_property(TARGET tcp_thserver PROPERTY CXX_STANDARD 11)
target_link_libraries(tcp_thserver pthread)
install(TARGETS tcp_thserver RUNTIME DESTINATION bin)
