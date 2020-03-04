cmake_minimum_required(VERSION 3.5)

project(lab02 LANGUAGES CXX)
project(lab02_th LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lab02 main.cpp)
add_executable(lab02_th main_threading.cpp)
