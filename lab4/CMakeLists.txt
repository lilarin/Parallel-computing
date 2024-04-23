cmake_minimum_required(VERSION 3.20)
project(lab4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Add source files
set(SERVER_SOURCES server.cpp)
set(CLIENT_SOURCES client.cpp)

# Create executables and link library to them
add_executable(server ${SERVER_SOURCES})
target_link_libraries(server PRIVATE ws2_32)

add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client PRIVATE ws2_32)
