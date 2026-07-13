#include <game/systems/Movement.hpp>

#include <algorithm>

#include <game/Entity.hpp>
#include <game/Geo.hpp>
#include <game/World.hpp>

using bt::game::Entity;
using bt::game::Geo;
using bt::game::Movement;
using bt::game::World;

namespace
{
    // Afterburner top-speed multiplier at full boost (GameRules.afterburnerBoost).
    constexpr auto AfterburnerBoost = 1.6F;
}

auto Movement::update(World& world, float dt) -> void
{
    for (auto& entity : world.entities())
    {
        advance(entity, dt);
    }
}

auto Movement::advance(Entity& entity, float dt) -> void
{
    // Afterburner: while held, boost lifts the top-speed ceiling proportionally
    // (topSpeed() reads speedBoost live).
    const auto boost = std::clamp(entity.commandedBoost, 0.0F, 1.0F);
    entity.speedBoost = 1.0F + ((AfterburnerBoost - 1.0F) * boost);

    // Speed: net acceleration is (throttle - brake) * maxAcceleration;
    // integrate and cap to [0, topSpeed].
    const auto throttle = std::clamp(entity.commandedThrottle, 0.0F, 1.0F);
    const auto brake = std::clamp(entity.commandedBrake, 0.0F, 1.0F);
    const auto accel = (throttle - brake) * entity.maxAcceleration;
    const auto newSpeed = std::clamp(entity.speed + (accel * dt), 0.0F, entity.topSpeed());
    entity.acceleration = dt > 0.0F ? (newSpeed - entity.speed) / dt : 0.0F;
    entity.speed = newSpeed;

    // Heading: steer (-1..1) is a fraction of the max turn rate.
    const auto steer = std::clamp(entity.commandedSteer, -1.0F, 1.0F);
    entity.heading = Geo::wrap360(entity.heading + (steer * entity.maxTurnRate * dt));

    // Position: integrate along the new heading on the Geo convention.
    entity.position = entity.position + Geo::offset(entity.heading, entity.speed * dt);
}
