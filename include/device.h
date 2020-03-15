#pragma once

#include "renderer.h"
#include "queue_family_indices.h"
#include "swapchain_details.h"

void selectGpu(struct Renderer *renderer, struct QueueFamilyIndices *indices, 
        struct SwapchainDetails *details);
void createDevice(const struct QueueFamilyIndices *indices, struct Renderer *renderer);