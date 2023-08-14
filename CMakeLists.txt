cmake_minimum_required(VERSION 3.15)

project(
  XJson
  VERSION 1.1.0
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(XJSON_EXAMPLE "build jsonPrintExample" ON)

file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/XJsonBase/*.c)
set(HEAD_DIR ${PROJECT_BINARY_DIR}/XJsonBase)

include_directories(${HEAD_DIR})

add_library(${PROJECT_NAME} STATIC ${SRC_LIST})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/XJsonBase/xjsonBase.h ${PROJECT_SOURCE_DIR}/XJson/include/xjsonBase.h)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libXJson.a ${PROJECT_SOURCE_DIR}/XJson/lib/libXJson.a)

if(XJSON_EXAMPLE)
  add_subdirectory(${PROJECT_SOURCE_DIR}/example)
endif()