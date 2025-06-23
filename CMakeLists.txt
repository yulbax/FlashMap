cmake_minimum_required(VERSION 3.31)
project(FlashMap CXX)

set(CMAKE_CXX_STANDARD 20)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-mavx2)
    add_definitions(-DUSE_AVX2)
endif()
add_executable(FlashMap main.cpp
        flashmap.hpp
        flashmap.tpp
        flashmapimpl.hpp
        flashmapiterator.hpp
        flashmapconcepts.hpp)



