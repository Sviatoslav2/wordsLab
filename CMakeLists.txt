cmake_minimum_required(VERSION 3.5.1)
project(CountWordsInATextFile)

set(CMAKE_CXX_STANDARD 11)
find_package(Threads)

set(SOURCE_FILES main.cpp)
add_executable(CountWordsInATextFile ${SOURCE_FILES})
target_link_libraries(CountWordsInATextFile Threads::Threads)