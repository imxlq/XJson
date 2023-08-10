set(XJSON_EXAMPLES
    jsonPrintExample
)

foreach(target ${XJSON_EXAMPLES})
  message(STATUS "example files found: ${target}.cc")
  add_executable(${target} EXCLUDE_FROM_ALL ${target}.cpp)
  target_include_directories(${target} PUBLIC ${PROJECT_SOURCE_DIR})
  target_link_libraries(${target} XJson)
endforeach()