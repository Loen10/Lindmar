#pragma once

struct Renderer *createRenderer();

void runRenderer(const struct Renderer *renderer);

void destroyRenderer(struct Renderer *renderer);