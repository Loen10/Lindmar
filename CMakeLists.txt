cmake_minimum_required(VERSION 3.16)

project(Lindmar C)

add_executable(Lindmar
        src/main.c
        src/renderer.c
        src/instance.c
        src/debug_messenger.c
        src/device.c
        src/swapchain_details.c
        src/queue_family_indices.c)

target_include_directories(Lindmar PUBLIC include)
target_link_directories(Lindmar PRIVATE lib)
target_link_libraries(Lindmar vulkan glfw3 dl pthread m X11)