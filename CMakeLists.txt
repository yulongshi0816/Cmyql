cmake_minimum_required(VERSION 3.28)
project(mysqlLearn)

set(CMAKE_CXX_STANDARD 17)

add_executable(mysqlLearn main.cpp
        CommonConnectionPool.h
        Connection.cpp
        public.h
        Connection.h
        CommonConnectionPool.cpp)

include_directories("/usr/local/mysql/include")
include_directories("/usr/local/mysql/lib")

target_link_libraries(mysqlLearn /usr/local/mysql/lib/libmysqlclient.dylib)


