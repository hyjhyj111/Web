cmake_minimum_required(VERSION 3.31)
project(web)

set(CMAKE_CXX_STANDARD 20)
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(SQLite3 REQUIRED)

add_executable(web main.cpp)

target_link_libraries(web PRIVATE SQLite::SQLite3)
