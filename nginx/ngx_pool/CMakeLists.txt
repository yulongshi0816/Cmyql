cmake_minimum_required(VERSION 3.28)
project(ngx_pool)

set(CMAKE_CXX_STANDARD 17)

add_executable(ngx_pool main.cpp
        ngx_mem_pool.h)
