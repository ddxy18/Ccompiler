cmake_minimum_required(VERSION 3.16)
project(CCompiler)

set(CMAKE_CXX_STANDARD 20)

include_directories(
        ../GoogleTest/googletest/include
        ../GoogleTest/googlemock/include
        ../include)

add_executable(CCompilerTest
        ast/list_util_test.cpp
        lex/lexer_test.cpp lex/nfa_test.cpp
        parser/parser_test.cpp
        )

add_subdirectory(../src ../src)
add_subdirectory(../GoogleTest ../GoogleTest)

target_link_libraries(CCompilerTest CCompilerLib)
target_link_libraries(CCompilerTest gtest_main)
target_link_libraries(CCompilerTest profiler)

enable_testing()