#pragma once

#include "renderer.h"
#include "queue_family_indices.h"
#include "swapchain_details.h"

void select_gpu(struct Renderer *renderer, struct QueueFamilyIndices *indices, 
        struct SwapchainDetails *details);
        
void create_device(const struct QueueFamilyIndices *indices, struct Renderer *renderer);