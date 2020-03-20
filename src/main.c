#include "renderer.h"

int main()
{
        struct Renderer *renderer = create_renderer();
        run_renderer(renderer);
        destroy_renderer(renderer);
}