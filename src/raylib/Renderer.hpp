#pragma once

#include <raylib.h>
#include <game/System.hpp>

namespace bt::raylib
{
    class Renderer : public bt::game::System
    {
    public:
        auto update(float deltaTime) -> void override;

    private:
    };
}