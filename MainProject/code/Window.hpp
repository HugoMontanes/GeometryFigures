

#pragma once

#include <SDL.h>
#include <string>
#include <utility>

namespace space
{

    class Window
    {
    public:

        enum Position
        {
            UNDEFINED = SDL_WINDOWPOS_UNDEFINED,
            CENTERED = SDL_WINDOWPOS_CENTERED,
        };

        struct OpenGL_Context_Settings
        {
            unsigned version_major = 3;         //Major OpenGL version
            unsigned version_minor = 3;         //Minor OpenGL version
            bool core_profile = true;           //Use the OpenGL core profile (modern OpenGL)
            unsigned depth_buffer_size = 24;    //Depth buffer precision in bits
            unsigned stencil_buffer_size = 0;   //Stencil buffer precision in bits
            bool enable_vsync = true;           //Enable vertical synchronization
        };

    private:

        SDL_Window* window_handle;      //Pointer to the SDL window
        SDL_GLContext opengl_context;   //Handle to the OpenGl renderign context

    public:

        Window(const char* title, int  left_x, int top_y, unsigned width, unsigned height, const OpenGL_Context_Settings& context_details);

        ~Window();      //Destructor

    public:

        //Prevents copying the Window to avoid issues with duplicates
        Window(const Window&) = delete;
        Window& operator = (const Window&) = delete;


        //Move constructor: Tranfers ownership of the SDL window and openGL context from other, leaving other in the empty state (nullptr)
        Window(Window&& other) noexcept
        {
            this->window_handle = std::exchange(other.window_handle, nullptr);
            this->opengl_context = std::exchange(other.opengl_context, nullptr);
        }

        //Move assignment and operator: Replaces the current opbject's resources with other's resources and cleaning if posible.
        Window& operator = (Window&& other) noexcept
        {
            this->window_handle = std::exchange(other.window_handle, nullptr);
            this->opengl_context = std::exchange(other.opengl_context, nullptr);
        }

    public:

        //Swaps front and back buffers of the openGL context, called at the end of rendering a frame to display the renderer image.
        void swap_buffers();
    };
}