cmake_minimum_required(VERSION 3.21)
project(lqp_proto)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(extern/googletest)

set(SOURCES main.cpp utils.hpp reverse_index.hpp fwd.hpp)
add_executable(lqp_proto ${SOURCES})

set(TEST_SOURCES reverse_index_test.cpp)
add_executable(lqp_proto_test ${TEST_SOURCES})
target_link_libraries(lqp_proto_test gtest gtest_main)