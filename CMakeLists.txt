cmake_minimum_required(VERSION 3.15)
project(lab1 C)

set(CMAKE_C_STANDARD 90)

add_executable(lab project/assembler.c project/firstPass.c project/secondPass.c project/secondPass.h project/input.c project/objects.h project/input.h project/firstPass.h)

