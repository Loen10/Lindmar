#pragma once

#define LAYER_COUNT 1u

void print_exit(const char *message);

#ifndef NDEBUG
void get_layers(const char *layers[LAYER_COUNT]);
#endif

/*
 * Debug: should be cleaned up by free()
 * Release: should be cleaned up by glfwTerminate()
 */
const char **create_instance_extensions(uint32_t *count);