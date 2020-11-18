cmake_minimum_required(VERSION 3.16)
project(CCompiler)

set(CMAKE_CXX_STANDARD 20)

add_library(Ast STATIC scope.cpp statement.cpp expression.cpp declaration.cpp type.cpp identifier.cpp)