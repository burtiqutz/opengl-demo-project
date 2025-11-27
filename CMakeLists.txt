cmake_minimum_required(VERSION 4.0)
project(opengl_demo_project)

set(CMAKE_CXX_STANDARD 20)

add_executable(opengl_demo_project main.cpp Mesh.cpp Model3D.cpp Shader.cpp stb_image.cpp tiny_obj_loader.cpp Camera.cpp Window.cpp)
target_link_libraries(opengl_demo_project glfw GL GLEW)
