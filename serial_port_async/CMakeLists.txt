cmake_minimum_required(VERSION 3.20)
project(
    serial_port_async
    LANGUAGES C
    VERSION 0.1
    DESCRIPTION "This is a small example project for a basic async serial port program."
)

add_executable(
    serial_port_async
    src/serial_port_async.c
    src/serial_port_async.h
)

target_compile_features(serial_port_async PRIVATE)

target_compile_options(serial_port_async PRIVATE -Wall -Wextra -Wpedantic)