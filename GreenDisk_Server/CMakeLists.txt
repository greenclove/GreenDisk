cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 11)
SET(PROJECT_NAME GreenDisk_Server)

set(SOURCE
        main.cpp
        src/MysqlConnect.h
        src/MysqlConnect.cpp
        utils/Utils.cpp
        dependence/easyloggingpp/easylogging++.cc)

include_directories(
        dependence/mysql/include
        dependence/easyloggingpp
        include/
        utils/)

link_directories(dependence/mysql/lib/)
link_libraries(libmysql)

project(${PROJECT_NAME})
add_executable(${PROJECT_NAME} ${SOURCE} Socket.cpp Socket.h)
target_link_libraries(${PROJECT_NAME} libmysql)