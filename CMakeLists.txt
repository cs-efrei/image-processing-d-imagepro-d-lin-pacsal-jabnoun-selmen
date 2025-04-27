cmake_minimum_required(VERSION 3.31)
project(Projet C)

set(CMAKE_C_STANDARD 11)

add_executable(Projet cmake-build-debug/main.c
        cmake-build-debug/bmp8.c
        cmake-build-debug/bmp24.h
        cmake-build-debug/bmp24.c
        cmake-build-debug/bmp8.h)
