cmake_minimum_required(VERSION 2.8.8)
project(project_coherence)

add_library(src "src/cache.h" "src/cache.cc" "src/main.cc")
add_subdirectory("val.v2")

add_executable(main "src/main.cc")
target_link_libraries(main src)

set(CMAKE_C_FLAGS "-std=c99")
