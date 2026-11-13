cmake_minimum_required(VERSION 3.20)
project(DmetProj2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_executable(DmetProj2 src/main.cpp src/Model.cpp)
target_include_directories(DmetProj2 PRIVATE external)

target_link_libraries(DmetProj2 PRIVATE
    glfw OpenGL::GL glm::glm assimp::assimp
)

target_compile_definitions(DmetProj2 PRIVATE
    PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)

if(APPLE)
    target_compile_definitions(DmetProj2 PRIVATE GL_SILENCE_DEPRECATION)
endif()