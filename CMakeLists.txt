cmake_minimum_required(VERSION 3.21)
project(lqp_proto)

set(CMAKE_CXX_STANDARD 20)

add_executable(lqp_proto main.cpp utils.hpp reverse_index.hpp fwd.hpp)
