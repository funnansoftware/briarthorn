#include <raylib/Window.hpp>

#include <stdexcept>

#include <raylib.h>

namespace bt::raylib
{
    Window::Window(const Traits& traits)
    {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(traits.width, traits.height, traits.title.c_str());

        if (!IsWindowReady())
        {
            throw std::runtime_error{"failed to create window"};
        }
    }

    Window::~Window()
    {
        CloseWindow();
    }
}
