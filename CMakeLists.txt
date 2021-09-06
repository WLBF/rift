cmake_minimum_required(VERSION 3.5)

project(rift)
set(CMAKE_CXX_STANDARD 17)

find_package(glog 0.6.0 REQUIRED)

file(GLOB rift_sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(rift STATIC ${rift_sources})

target_link_libraries(rift glog::glog)

target_include_directories(rift PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(test4 test/test4.cpp)
target_link_libraries(test4 rift glog::glog)
add_test(NAME Test4 COMMAND test4)

enable_testing()
find_package(GTest REQUIRED)
add_executable(event_loop_test test/event_loop_test.cpp)
target_link_libraries(event_loop_test rift GTest::GTest GTest::Main glog::glog)
add_test(NAME EventLoopTest COMMAND event_loop_test)

