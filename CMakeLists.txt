cmake_minimum_required(VERSION 3.20)
project(DmetProj2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(DmetProj2 src/main.cpp)

target_link_libraries(DmetProj2 PRIVATE glfw OpenGL::GL)

if(APPLE)
    target_compile_definitions(DmetProj2 PRIVATE GL_SILENCE_DEPRECATION)
endif()