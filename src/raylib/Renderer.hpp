#pragma once

#include <raylib.h>
#include <game/System.hpp>
#include <optional>
#include <raylib/Window.hpp>

namespace bt::raylib
{
    struct Triangle
    {
        Vector2 top;
        Vector2 left;
        Vector2 right;
    };

    class Renderer : public bt::game::System
    {
    public:
        Renderer();

        auto update(float deltaTime) -> void override;

    private:
        std::optional<Window> window_;
    };
}