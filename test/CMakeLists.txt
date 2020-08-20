cmake_minimum_required(VERSION 3.16)
project(Ccompiler)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(../GoogleTest ../GoogleTest)

include_directories(../GoogleTest/googletest/include
        ../GoogleTest/googlemock/include
        ../include)

add_executable(CcompilerTest ../src/lex/nfa.cpp ../src/lex/token.cpp ../src/lex/lex.cpp lex/token_test.cpp lex/lex_test.cpp lex/nfa_test.cpp)

target_link_libraries(CcompilerTest gtest_main)