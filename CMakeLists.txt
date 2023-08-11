cmake_minimum_required(VERSION 3.5)

project(
  XJson
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

option(XJSON_EXAMPLE "build jsonPrintExample" ON)

message(STATUS "XJSON_BASE_BUILD: ${XJSON_BASE_BUILD}")

file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/jsonBase/*.c)
set(HEAD_DIR ${PROJECT_BINARY_DIR}/jsonBase)

include_directories(${HEAD_DIR})

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})

if(XJSON_EXAMPLE)
  add_subdirectory(${PROJECT_SOURCE_DIR}/example)
endif()