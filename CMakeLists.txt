cmake_minimum_required(VERSION 3.16)
project(CCompiler)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(CCompiler src/main.cpp)

# for temporary experiments
add_executable(CCompilerTemp temp/temp.cpp)

add_executable(CCompilerSource test/source.c)

add_subdirectory(test)

target_link_libraries(CCompiler CCompilerLib)
target_link_libraries(CCompiler profiler)