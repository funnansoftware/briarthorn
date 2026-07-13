#pragma once

#include <raylib.h>
#include <game/System.hpp>
#include <memory>
#include <string>
#include <vector>

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

        auto addSystem(std::unique_ptr<bt::game::System> x) -> void;

        // Opens the window and drives the render loop until it is closed.
        // Throws std::runtime_error if the window cannot be created.
        auto run() const -> void;

    private:
        std::vector<std::unique_ptr<bt::game::System>> systems_;
        WindowSize size_;
        std::string title_;
    };

}
