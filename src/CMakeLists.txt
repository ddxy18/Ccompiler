cmake_minimum_required(VERSION 3.16)
project(CCompiler)

set(CMAKE_CXX_STANDARD 20)

add_library(CCompilerLib STATIC environment.cpp)

#add_subdirectory(ast)
add_subdirectory(lex)
#add_subdirectory(parser)

target_link_libraries(CCompilerLib
        #[[Ast]]
        Lex
        #[[Parser]])
