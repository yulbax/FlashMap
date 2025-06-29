cmake_minimum_required(VERSION 3.31)
project(FlashMap CXX)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(CHECK_KEY_EQUALITY)

add_executable(FlashMap main.cpp
        flashmap.hpp
        flashmap.tpp
        flashmapimpl.hpp
        flashmapiterator.hpp
        flashmapconcepts.hpp
        listallocator.hpp)