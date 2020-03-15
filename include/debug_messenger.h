#pragma once
#ifndef NDEBUG
#include "renderer.h"

void createDebugMessenger(struct Renderer *renderer);
void destroyDebugMessenger(const struct Renderer *renderer);
#endif