cmake_minimum_required(VERSION 3.20)
project(
    serial_port_pthread
    LANGUAGES C
    VERSION 0.1
    DESCRIPTION "This is a small example project for a basic async serial port program."
)

add_executable(
    serial_port_pthread
    src/serial_port_pthread.c
    src/serial_port_pthread.h
)

target_compile_features(serial_port_pthread PRIVATE)

target_compile_options(serial_port_pthread PRIVATE -Wall -Wextra -Wpedantic)