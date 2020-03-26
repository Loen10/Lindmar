#pragma once

struct Renderer;

// Should be cleaned up by destroy_renderer()
struct Renderer *create_renderer(void);

void run_renderer(struct Renderer *renderer);

void destroy_renderer(struct Renderer *renderer);