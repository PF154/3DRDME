cmake_minimum_required(VERSION 3.20)
project(rw_sim LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(CUDAToolkit REQUIRED)

add_executable(rw_sim
    src/main.cu
    src/Simulation.cu
)

target_include_directories(rw_sim PRIVATE include)

set_target_properties(rw_sim PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

target_link_libraries(rw_sim
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
    CUDA::cudart
)
