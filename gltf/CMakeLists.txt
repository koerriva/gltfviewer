aux_source_directory(src GLTF_SRC)
include_directories(include)
include_directories(../glad/include)
add_library(gltf ${GLTF_SRC})
target_link_libraries(gltf glad glfw3 opengl32)