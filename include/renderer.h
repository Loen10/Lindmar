#pragma once

struct Renderer;

void run_renderer(const struct Renderer *renderer);

void destroy_renderer(struct Renderer *renderer);

/* Should be cleaned up by destroy_renderer() */
struct Renderer *create_renderer(void);