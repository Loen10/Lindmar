#pragma once

#ifndef NDEBUG
#define LAYER_COUNT 1
extern const char* const LAYERS[];
#endif

void assert_layers_support();

/* Should be cleaned up by free() */
const char **get_instance_extensions();