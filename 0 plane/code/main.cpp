

#include "Scene.hpp"
#include "Window.hpp"

using space::Window;

int main(int, char* [])
{
    constexpr unsigned viewport_width = 1024;
    constexpr unsigned viewport_height = 576;

    Window window("Plane example", Window::Position::CENTERED, Window::Position::CENTERED, viewport_width, viewport_height, { 3,3 });

    return 0;
}



