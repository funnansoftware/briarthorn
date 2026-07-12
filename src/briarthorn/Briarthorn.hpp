#pragma once

#include <string>

#include <raylib.h>

namespace bt
{
    struct WindowSize
    {
        int width;
        int height;
    };

    struct Triangle
    {
        Vector2 top;
        Vector2 left;
        Vector2 right;
    };

    class Briarthorn
    {
    public:
        Briarthorn(WindowSize size, std::string title);

        [[nodiscard]] auto size() const -> WindowSize;
        [[nodiscard]] auto title() const -> const std::string &;

        // The triangle drawn each frame: apex in the upper half, horizontal base
        // across the middle, centered and scaled to the window.
        [[nodiscard]] auto triangle() const -> Triangle;

        // Opens the window and drives the render loop until it is closed.
        // Throws std::runtime_error if the window cannot be created.
        auto run() const -> void;

    private:
        WindowSize size_;
        std::string title_;
    };

}
