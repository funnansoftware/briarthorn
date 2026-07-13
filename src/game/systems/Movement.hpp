#pragma once

#include <game/System.hpp>

namespace bt::game
{
    struct Entity;

    /// @brief The kinematic engine. Each step it advances every entity from its control
    /// inputs (commandedThrottle/Brake/Steer/Boost) within the limits its
    /// capabilities afford. Ports `lib/sim/systems/system_movement.dart`.
    class Movement : public System
    {
    public:
        auto update(World& world, float dt) -> void override;

    private:
        static auto advance(Entity& entity, float dt) -> void;
    };
}
