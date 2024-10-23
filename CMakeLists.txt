#set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/cmake_target)
message(STATUS "exe out:" ${EXECUTABLE_OUTPUT_PATH})
message(STATUS "binary:" ${CMAKE_BINARY_DIR})
message(STATUS "src:" ${CMAKE_SOURCE_DIR})
SET(CMAKE_BUILD_TYPE "Debug")

# if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
#     message(FATAL_ERROR "Prevented in-tree build. This is bad practice.")
    
# endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

cmake_minimum_required(VERSION 3.15)
project(yshell)
add_executable(yshell shell.c built_in.c)

