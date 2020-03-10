

#include "renderer.h"

int main()
{
        Renderer renderer;
        createRenderer(&renderer);
        runRenderer(renderer);
        destroyRenderer(renderer);
}