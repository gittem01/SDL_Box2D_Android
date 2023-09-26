#include "SDL.h"
#include "box2d/box2d.h"
#include <android/log.h>
#include <Base.h>

void renderSquare(SDL_Renderer *renderer)
{
    static int p = 0;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_Rect rect = { p++, 0, 100, 100 };
    SDL_RenderFillRect(renderer, &rect);
}

int main(int argc, char *argv[])
{
    Base base;

    base.loop();

    return 0;
}
