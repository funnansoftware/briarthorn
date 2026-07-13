#include <raylib/Window.hpp>

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

auto Window::shouldClose() const -> bool
{
    return WindowShouldClose();
}

auto Window::begin(Color clear) const -> void
{
    BeginDrawing();
    ClearBackground(clear);
}

auto Window::end() const -> void
{
    EndDrawing();
    PollInputEvents();
    SwapScreenBuffer();
}

auto Window::size() const -> Vector2
{
    const auto w = static_cast<float>(GetScreenWidth());
    const auto h = static_cast<float>(GetScreenHeight());

    return Vector2{.x = w, .y = h};
}