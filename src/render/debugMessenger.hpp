#pragma once

#include <vulkan/vulkan.hpp>

namespace lmar::render::debugMessenger
{
    vk::UniqueDebugUtilsMessengerEXT create(const vk::UniqueInstance& instance);
}