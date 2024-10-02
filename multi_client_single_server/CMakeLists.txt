cmake_minimum_required(VERSION 3.20)
project(
    multi_client_single_server
    LANGUAGES C
    VERSION 0.1
    DESCRIPTION "A basic example of a single server which pushes messages to multiple clients."
)

add_executable(
    fifo_server
    src/fifo_server.c
    src/fifo_common.h
    src/linked_list.c
    src/linked_list.h
)

add_executable(
    fifo_client
    src/fifo_client.c
    src/fifo_common.h
    src/linked_list.c
    src/linked_list.h
)

target_compile_features(fifo_server PRIVATE)
target_compile_features(fifo_client PRIVATE)

target_compile_options(fifo_server PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(fifo_client PRIVATE -Wall -Wextra -Wpedantic)