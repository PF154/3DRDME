cmake_minimum_required(VERSION 3.20)
project(rw_sim)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_executable(rw_sim src/main.cpp src/SimData.cpp)

target_include_directories(rw_sim PRIVATE include)

target_link_libraries(rw_sim
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)

set_target_properties(rw_sim PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
