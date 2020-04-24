cmake_minimum_required(VERSION 3.13)
project(hw1)

set(CMAKE_CXX_STANDARD 14)

add_executable(hw1 Commands.cpp Commands.h signals.cpp signals.h
        smash.cpp )