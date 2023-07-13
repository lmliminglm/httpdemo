cmake_minimum_required(VERSION 3.10)
project(abup_log C CXX)

file(GLOB_RECURSE C_HEADER_FILE ${PROJECT_SOURCE_DIR}/inc/*.h)
file(GLOB_RECURSE C_SOURCE_FILE ${PROJECT_SOURCE_DIR}/src/*.c ${PROJECT_SOURCE_DIR}/zlog/src/*.c)

# Add a library with the above sources
add_library(${PROJECT_NAME} STATIC ${C_SOURCE_FILE})
add_library(foundation::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE 
                           ${CMAKE_BINARY_DIR}/src/
                           ${PROJECT_SOURCE_DIR}/inc
                           ${CMAKE_SOURCE_DIR}/src/foundation/abup_log/zlog/inc/
                           ${CMAKE_SOURCE_DIR}/src/foundation/libfile/inc/
                           ${CMAKE_SOURCE_DIR}/src/foundation/os/inc/)

target_link_libraries(${PROJECT_NAME} PRIVATE
                      ${THREAD_LIBRARY})

# install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
# install(FILES ${C_HEADER_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})