project(matio C CXX)

cmake_minimum_required(VERSION 2.6)

file(GLOB matio_srcs "src/*.c")

add_library(matio SHARED ${matio_srcs})
add_library(matio-static STATIC ${matio_srcs})

target_include_directories(matio PRIVATE "${CMAKE_SOURCE_DIR}/visual_studio")
target_include_directories(matio PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories(matio-static PRIVATE "${CMAKE_SOURCE_DIR}/visual_studio")
target_include_directories(matio-static PRIVATE "${CMAKE_SOURCE_DIR}/src")

add_executable(testMatio testMatio.cpp)
target_link_libraries(testMatio matio-static)
target_include_directories(testMatio PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories(testMatio PRIVATE "${CMAKE_SOURCE_DIR}/visual_studio")
