cmake_minimum_required(VERSION 3.17)

project(Lindmar CXX)

add_executable(Lindmar
    src/Main.cpp
    src/render/Renderer.cpp
    src/render/debugMessenger.cpp
    src/render/instance.cpp
    src/render/window.cpp)

target_compile_features(Lindmar PUBLIC cxx_std_17)
target_include_directories(Lindmar PUBLIC src)
target_link_directories(Lindmar PRIVATE lib)
target_link_libraries(Lindmar vulkan glfw3 dl pthread m X11)