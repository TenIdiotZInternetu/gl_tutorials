# Set the minimum CMake version
cmake_minimum_required(VERSION 3.10)

project(hw_ssao)

add_executable(hw_ssao 
	main.cpp
	myrenderer.hpp
	shader.h
	ssao.hpp
)
target_sources(hw_ssao PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/../glad/src/glad.c
)
target_link_libraries(hw_ssao utils glm::glm glfw OpenGL::GL)
target_include_directories(hw_ssao PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../glad/include
	${CMAKE_CURRENT_SOURCE_DIR}/../utils
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
)

execute_process(
    COMMAND ln -s ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
)

execute_process(
    COMMAND ln -s ${CMAKE_CURRENT_SOURCE_DIR}/../data ${CMAKE_CURRENT_BINARY_DIR}/data
)
