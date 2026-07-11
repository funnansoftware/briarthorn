#include "Game.hpp"

#include <string>
#include <utility>

#include <raylib.h>

namespace briarthorn
{

    Game::Game(WindowSize size, std::string title) : size_{size}, title_{std::move(title)}
    {
    }

    auto Game::size() const -> WindowSize
    {
        return size_;
    }

    auto Game::title() const -> const std::string &
    {
        return title_;
    }

    auto Game::triangle() const -> Triangle
    {
        constexpr float horizontalCenterDivisor = 2.0F;
        constexpr float apexHeightDivisor = 6.0F;
        constexpr float baseHeightDivisor = 2.0F;
        constexpr float baseHalfWidthDivisor = 8.0F;

        const auto width = static_cast<float>(size_.width);
        const auto height = static_cast<float>(size_.height);

        const float centerX = width / horizontalCenterDivisor;
        const float baseY = height / baseHeightDivisor;

        return Triangle{
            .top = {.x = centerX, .y = height / apexHeightDivisor},
            .left = {.x = centerX - (width / baseHalfWidthDivisor), .y = baseY},
            .right = {.x = centerX + (width / baseHalfWidthDivisor), .y = baseY},
        };
    }

    auto Game::run() const -> void
    {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(size_.width, size_.height, title_.c_str());

        const Triangle shape = triangle();

        while (!WindowShouldClose())
        {
            BeginDrawing();
            ClearBackground(GRAY);
            DrawTriangle(shape.top, shape.left, shape.right, ORANGE);
            EndDrawing();
            PollInputEvents();
            SwapScreenBuffer();
        }

        CloseWindow();
    }

} // namespace briarthorn
