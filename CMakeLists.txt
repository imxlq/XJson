cmake_minimum_required(VERSION 3.5)

project(
  XJson
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lib/libXJson.a)
  option(XJSON_BASE_BUILD "build jsonBase" ON)
endif()

option(XJSON_EXAMPLE "build jsonPrintExample" ON)

message(STATUS "XJSON_BASE_BUILD: ${XJSON_BASE_BUILD}")

# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/jsonBase/*.c)
set(HEAD_DIR ${PROJECT_BINARY_DIR}/jsonBase)

include_directories(${HEAD_DIR})

if(XJSON_BASE_BUILD)
  add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
endif()

if(XJSON_EXAMPLE)
  add_subdirectory(${PROJECT_SOURCE_DIR}/example)
endif()