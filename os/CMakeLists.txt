cmake_minimum_required(VERSION 3.10)
project (admos C CXX)

file(GLOB_RECURSE C_HEADER_FILE ${PROJECT_SOURCE_DIR}/inc/*.h)
file(GLOB_RECURSE C_SOURCE_FILE ${PROJECT_SOURCE_DIR}/src/*.c
                                ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_definitions(-D_GNU_SOURCE)
add_definitions(-DADB_DBG_MEM)


add_library(${PROJECT_NAME} STATIC ${C_SOURCE_FILE})
add_library(foundation::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE 
                             ${PROJECT_SOURCE_DIR}/inc)

