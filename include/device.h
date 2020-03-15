#pragma once

#include "renderer.h"

struct QueueFamilyIndices {
        int graphics;
        int present;
};

struct SwapchainDetails {
        uint32_t surfaceFormatCount;
        uint32_t presentModeCount;
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR *surfaceFormats;
        VkPresentModeKHR *presentModes;
};

void selectGpu(struct Renderer *renderer, struct QueueFamilyIndices *indices, 
        struct SwapchainDetails *details);
void createDevice(const struct QueueFamilyIndices *indices, struct Renderer *renderer);