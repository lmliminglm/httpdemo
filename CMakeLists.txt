cmake_minimum_required(VERSION 3.10)

project(httpdemo C CXX)

set(SYSTEM_LIBRARY pthread rt dl)
set(THREAD_LIBRARY pthread)

add_subdirectory(os)
add_subdirectory(cjson)

# 添加源文件
add_executable(httpdemo httpdemo.c)

#添加依赖的库文件路径
target_link_libraries(${PROJECT_NAME} PRIVATE
                        foundation::admos
                        foundation::cjson
                        ${CMAKE_SOURCE_DIR}/curl/lib/x86/libcurl-d.a
                        ${CMAKE_SOURCE_DIR}/openssl/lib/x86/libssl-static.a
                        ${CMAKE_SOURCE_DIR}/openssl/lib/x86/libcrypto-static.a
                        ${THREAD_LIBRARY}
                        ${SYSTEM_LIBRARY}
                        ${THIRD_LIBRARY}
)

#添加依赖的头文件路径
target_include_directories(${PROJECT_NAME} PRIVATE
                            ${CMAKE_SOURCE_DIR}/os/inc/
                            ${CMAKE_SOURCE_DIR}/cjson/inc/ 
                            ${CMAKE_SOURCE_DIR}/curl/inc/
                            ${CMAKE_SOURCE_DIR}/openssl/inc/  
)

