cmake_minimum_required(VERSION 3.5)

project(lab02_self LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
add_executable(lab02_self main.cpp)
target_link_libraries(lab02_self Threads::Threads)
