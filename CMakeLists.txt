cmake_minimum_required(VERSION 3.21)
project(gltfviewer)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(glad)
add_subdirectory(gltf)

include_directories(glad/include)
include_directories(gltf/include)

add_executable(gltfviewer main.cpp)

target_link_libraries(gltfviewer glad gltf)

file(COPY data DESTINATION .)
