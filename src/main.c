#include "renderer.h"

int main()
{
        struct Renderer *renderer = createRenderer();
        runRenderer(renderer);
        destroyRenderer(renderer);
}