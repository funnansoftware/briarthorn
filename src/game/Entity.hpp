#pragma once

#include <cstdint>

#include <game/Vec2.hpp>

namespace bt::game
{
    /// Stable handle to an entity. 0 is the null handle (no entity); the world
    /// hands out ids from 1 up, so a stale handle never silently aliases a live
    /// entity the way a reused vector index would.
    using EntityId = std::uint32_t;

    inline constexpr auto NullEntity = EntityId{0};

    // Default capability ceilings for a bare entity. Flat values for now; the
    // full port derives these from stats via GameRules.
    inline constexpr auto DefaultTopSpeed = 180.0F;        // m/s
    inline constexpr auto DefaultTurnRate = 90.0F;         // deg/s
    inline constexpr auto DefaultMaxAcceleration = 120.0F; // m/s^2

    /// Ground-truth state for one moving object in the world: a position, a
    /// heading, the kinematic state, and the control inputs a controller sets
    /// (through the command buffer) for the movement system to integrate. Pure
    /// state, no behaviour — systems write these, the renderer reads them. A
    /// trimmed port of `lib/sim/model/entity.dart`: kinematics only for now, with
    /// stats, weapons and abilities still to come.
    struct Entity
    {
        EntityId id{NullEntity};

        // Kinematic state — the movement system writes these.
        Vec2 position{};
        float heading{0.0F};      // degrees clockwise from north, [0, 360)
        float speed{0.0F};        // m/s
        float acceleration{0.0F}; // m/s^2 applied last tick (output only)

        // Control inputs — the command buffer writes these, movement reads them.
        float commandedThrottle{0.0F}; // 0..1
        float commandedBrake{0.0F};    // 0..1
        float commandedSteer{0.0F};    // -1..1
        float commandedBoost{0.0F};    // 0..1 (afterburner intent)
        float speedBoost{1.0F};        // live top-speed multiplier

        // Capability ceilings the movement system reads. Flat fields for now; the
        // full port derives these from stats via GameRules (Entity.topSpeed etc).
        float topSpeedBase{DefaultTopSpeed}; // m/s at speedBoost == 1
        float maxTurnRate{DefaultTurnRate};  // deg/s
        float maxAcceleration{DefaultMaxAcceleration};

        float health{100.0F};

        /// Top speed right now, with the afterburner multiplier folded in.
        [[nodiscard]] auto topSpeed() const -> float;
    };
}
