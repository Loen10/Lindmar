#pragma once

typedef struct Renderer_T *Renderer;

/*
 * Should be cleaned up by destroyRenderer()
 */
Renderer createRenderer();

void runRenderer(const Renderer renderer);

void destroyRenderer(const Renderer renderer);