cmake_minimum_required(VERSION 3.5)

project(rift)
set(CMAKE_CXX_STANDARD 17)

find_package(glog 0.6.0 REQUIRED)
find_package(GTest REQUIRED)

file(GLOB rift_sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(rift STATIC ${rift_sources})

target_link_libraries(rift glog::glog)

target_include_directories(rift PUBLIC ${PROJECT_SOURCE_DIR}/include)

enable_testing()
add_subdirectory(test)
