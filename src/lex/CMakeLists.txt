cmake_minimum_required(VERSION 3.16)
project(CCompiler)

set(CMAKE_CXX_STANDARD 20)

add_library(Lex STATIC lexer.cpp nfa.cpp ../environment.cpp)