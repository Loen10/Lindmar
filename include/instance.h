#pragma once

#include "renderer.h"

#ifndef NDEBUG
#define LAYER_COUNT 1
extern const char* const LAYERS[];
#endif

void createInstance(struct Renderer *renderer);
const char **getInstanceExtensions();
#ifndef NDEBUG
void assertLayersSupport();
#endif