#include "renderer.h"

int main()
{
        Renderer renderer = createRenderer();
        runRenderer(renderer);
        destroyRenderer(renderer);
}