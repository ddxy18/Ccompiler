cmake_minimum_required(VERSION 3.16)
project(CCompiler)

set(CMAKE_CXX_STANDARD 20)

include_directories(
        ../GoogleTest/googletest/include
        ../GoogleTest/googlemock/include
        ../include)

add_executable(CCompilerTest
        lex/lexer_test.cpp lex/nfa_test.cpp
        )

target_link_libraries(CCompilerTest Lex)
target_link_libraries(CCompilerTest gtest_main)

enable_testing()