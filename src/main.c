#include "renderer.h"

int main(void)
{
    struct Renderer *renderer = create_renderer();
    run_renderer(renderer);
    destroy_renderer(renderer);
}