#include "renderer.h"

int main()
{
        struct Renderer renderer;
        createRenderer(&renderer);
        runRenderer(&renderer);
        destroyRenderer(&renderer);
}