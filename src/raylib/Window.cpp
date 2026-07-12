#include <raylib/Window.hpp>

#include <raylib.h>

using bt::raylib::Window;

Window::Window(const Traits& traits) : title_(traits.title), width_(traits.width), height_(traits.height)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width_, height_, title_.c_str());
}

Window::~Window()
{
    CloseWindow();
}