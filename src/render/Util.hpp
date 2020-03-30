#pragma once

#include <vulkan/vulkan.h>

namespace lmar::render
{
    void AssertVulkan(VkResult res, const char* msg);
}