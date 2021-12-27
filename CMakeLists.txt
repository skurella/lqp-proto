cmake_minimum_required(VERSION 3.21)
project(lqp_proto)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(extern/googletest)

file(GLOB SOURCES src/*.cpp src/*.hpp)
add_executable(lqp_proto ${SOURCES})

file(GLOB TEST_SOURCES test/*.cpp test/*.hpp)
add_executable(lqp_proto_test ${TEST_SOURCES})
target_include_directories(lqp_proto_test PRIVATE src)
target_link_libraries(lqp_proto_test gtest gtest_main)
