#include <stdexcept>

#include "util.hpp"

using namespace lmar::render;

void util::assertVulkan(VkResult res, const char* msg)
{
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error{msg};
    }
}

