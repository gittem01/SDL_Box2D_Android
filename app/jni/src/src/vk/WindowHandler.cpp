#include <vk/WindowHandler.h>
#include <vk/Camera.h>

WindowHandler::WindowHandler(Camera* cam)
{
    this->cam = cam;
    this->massInit();
}

void WindowHandler::handleMouseData()
{
    this->mouseData[5] = 0;
    this->trackpadData[0] = 0;
    this->trackpadData[1] = 0;
    for (int i = 2; i < 5; i++)
    {
        if (this->mouseData[i] == 2)
        {
            this->mouseData[i] = 1;
        }
        if (releaseQueue[i - 2])
        {
            mouseData[i] = 0;
            releaseQueue[i - 2] = false;
        }
    }
    if (lastMousePos[0] == mouseData[0] && lastMousePos[1] == mouseData[1])
    {
        mouseData[6] = 0;
    }
    else
    {
        mouseData[6] = 1;
    }
}

void WindowHandler::handleKeyData()
{
    for (int i : newPressIndices)
    {
        keyData[i] = 1;
    }
    newPressIndices.clear();
}

bool WindowHandler::looper()
{
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    handleMouseData();
    handleKeyData();

    //glfwPollEvents();

    bool done = false;

    return done;
}

void WindowHandler::massInit()
{
    SDL_DisplayMode displayMode;
    bool r = SDL_GetDisplayMode(0, 0, &displayMode);

    int refreshRate;
    if (displayMode.refresh_rate > 0 && r == 0)
    {
        refreshRate = displayMode.refresh_rate;
    }
    else
    {
        refreshRate = 60;
    }
    float deltaTime = 1.0f / (float)refreshRate;

    window = SDL_CreateWindow("", 0, 0, 0, 0, SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED);

    int x1, y1, x2, y2;
    SDL_GetWindowSize(window, &x1, &y1);
    SDL_GetWindowSizeInPixels(window, &x2, &y2);

    dpiScaling = std::round(x2 / x1);

    mouseData[0] = windowSizes.x / 2.0f;
    mouseData[1] = windowSizes.y / 2.0f;
}