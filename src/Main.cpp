#include <iostream>

#include "render/Renderer.hpp"

using namespace lmar::render;

int main()
{
    try
    {
        Renderer renderer;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}