#pragma once

#include "renderer.h"

#ifndef NDEBUG
#define LAYER_COUNT 1
extern const char* const LAYERS[];
#endif

void create_instance(struct Renderer *renderer);

void assert_layers_support();

const char **get_instance_extensions();