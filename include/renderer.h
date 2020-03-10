#pragma once

typedef struct Renderer_T *Renderer;

void createRenderer(Renderer* renderer);

void runRenderer(const Renderer renderer);

void destroyRenderer(const Renderer renderer);