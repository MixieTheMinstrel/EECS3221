cmake_minimum_required(VERSION 3.13)
project(Assignment2 C)

set(CMAKE_C_STANDARD 11)

add_executable(Assignment2 question2.c question1.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")