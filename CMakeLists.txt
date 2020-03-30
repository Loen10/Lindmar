cmake_minimum_required(VERSION 3.17)

project(Lindmar CXX)

add_executable(
    Lindmar
    src/Main.cpp
    src/render/Renderer.cpp
    src/render/Window.cpp
    src/render/Instance.cpp
    src/render/Util.cpp
    src/render/DebugMessenger.cpp
)

target_compile_features(Lindmar PUBLIC cxx_std_17)
target_include_directories(Lindmar PUBLIC include)
target_link_directories(Lindmar PRIVATE lib)
target_link_libraries(Lindmar vulkan glfw3 dl pthread m X11)