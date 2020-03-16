#pragma once
#ifndef NDEBUG
#include "renderer.h"

void create_debug_messenger(struct Renderer *renderer);

void destroy_debug_messenger(const struct Renderer *renderer);
#endif