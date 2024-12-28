#include "Scene.hpp"
#include "Window.hpp"

using space::Window;

int main(int, char* [])
{
    constexpr unsigned viewport_width = 1024;
    constexpr unsigned viewport_height = 576;

    Window window("Plane example", Window::Position::CENTERED, Window::Position::CENTERED, viewport_width, viewport_height, { 3,3 });

    space::Scene scene(1920, 1080);

    bool running = true;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_WINDOWEVENT)
            {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    scene.resize(event.window.data1, event.window.data2);
                }
            }
        }

        scene.update();
        scene.render();
        window.swap_buffers();
    }

    return 0;
}




