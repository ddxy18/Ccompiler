cmake_minimum_required(VERSION 3.16)
project(CCompiler)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")

include_directories(include)

add_executable(CCompiler src/main.cpp)

# for temporary experiments
add_executable(CCompilerTemp temp/temp.cpp)

add_subdirectory(GoogleTest GoogleTest)
add_subdirectory(src)
add_subdirectory(test)

target_link_libraries(CCompiler CCompilerLib)