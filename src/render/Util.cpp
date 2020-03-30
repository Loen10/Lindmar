#include <stdexcept>

#include "Util.hpp"

using namespace lmar;

void render::AssertVulkan(VkResult res, const char* msg)
{
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error{msg};
    }
}
