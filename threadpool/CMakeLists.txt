cmake_minimum_required(VERSION 3.28)
project(threadpool)

set(CMAKE_CXX_STANDARD 20)

add_executable(threadpool main.cpp
        threadpool.h
        threadpool.cpp)
