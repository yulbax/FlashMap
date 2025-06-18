cmake_minimum_required(VERSION 4.0)
project(FlashMap CXX)

set(CMAKE_CXX_STANDARD 23)

add_executable(FlashMap main.cpp
        flashmap.hpp
        flashmap.tpp
        flashmapimpl.hpp
        flashmapiterator.hpp)



