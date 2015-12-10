cmake_minimum_required(VERSION 2.6)
project(multimilter)

add_executable(multimilter main.cpp thpool.cpp milter_packet.cpp miltertalk.cpp)
set_property(TARGET multimilter PROPERTY CXX_STANDARD 11)
target_link_libraries(multimilter pthread)
install(TARGETS multimilter RUNTIME DESTINATION bin)
