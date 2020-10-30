cmake_minimum_required(VERSION 3.0)
project(opengl-imgui-sample CXX)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

SET(CMAKE_CXX_STANDARD 17)


# CONFIG option is important so that CMake doesnt search for modules into the default modules directory
find_package(imgui CONFIG)
find_package(glfw CONFIG)
find_package(glew CONFIG)
find_package(fmt CONFIG)
find_package(glm CONFIG)
find_package(stb CONFIG)
find_package(tinyobjloader CONFIG)

add_executable( toric_earth_run
                main.cpp
                environment.h
                textures.h
                opengl_shader.cpp
                opengl_shader.h
                torus.h
                bindings/imgui_impl_glfw.cpp
                bindings/imgui_impl_opengl3.cpp
                bindings/imgui_impl_glfw.h
                bindings/imgui_impl_opengl3.h
                shaders/environment.vs
                shaders/environment.fs
                shaders/obj.vs
                shaders/obj.fs
                shaders/torus.vs
                shaders/torus.fs
)

add_custom_command(TARGET toric_earth_run
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/shaders/environment.vs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/shaders/environment.fs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/shaders/obj.vs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/shaders/obj.fs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/shaders/torus.vs ${PROJECT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/shaders/torus.fs ${PROJECT_BINARY_DIR}
)

target_compile_definitions(toric_earth_run PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(toric_earth_run imgui::imgui GLEW::glew_s glfw::glfw fmt::fmt glm::glm stb::stb tinyobjloader::tinyobjloader)
