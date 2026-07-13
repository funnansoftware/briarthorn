#include <raylib/Renderer.hpp>

using bt::raylib::Renderer;

Renderer::Renderer()
{
    window_.emplace(bt::raylib::Window::Traits{
        .title = "Briarthorn",
        .width = 800,
        .height = 600,
    });
}

auto Renderer::update([[maybe_unused]] float deltaTime) -> void
{
    if (!window_)
    {
        return;
    }

    if (window_->shouldClose())
    {
        window_.reset();
        return;
    }

    constexpr auto horizontalCenterDivisor = 2.0F;
    constexpr auto apexHeightDivisor = 6.0F;
    constexpr auto baseHeightDivisor = 2.0F;
    constexpr auto baseHalfWidthDivisor = 8.0F;

    const auto size = window_->size();
    const auto width = size.x;
    const auto height = size.y;

    const float centerX = width / horizontalCenterDivisor;
    const float baseY = height / baseHeightDivisor;

    const auto shape = Triangle{
        .top = {.x = centerX, .y = height / apexHeightDivisor},
        .left = {.x = centerX - (width / baseHalfWidthDivisor), .y = baseY},
        .right = {.x = centerX + (width / baseHalfWidthDivisor), .y = baseY},
    };

    window_->begin(GRAY);
    DrawTriangle(shape.top, shape.left, shape.right, ORANGE);
    window_->end();
}