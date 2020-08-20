cmake_minimum_required(VERSION 3.16)
project(Ccompiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(test)

add_executable(Ccompiler src/main.cpp src/lex/token.cpp src/lex/nfa.cpp src/lex/lex.cpp src/main.cpp)

# for temporary experiments
add_executable(CcompilerTemp temp/temp.cpp)