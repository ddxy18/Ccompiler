cmake_minimum_required(VERSION 3.16)
project(CCompiler)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(../ast ../ast)

add_library(Parser STATIC parser.cpp)

target_link_libraries(Parser
        Ast
        )