cmake_minimum_required(VERSION 3.25)

project(MuMain LANGUAGES CXX)

# Optional: set global C++ standard as fallback
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory("src")
