#include <game/Geo.hpp>

#include <cmath>
#include <numbers>

namespace bt::game
{
    namespace
    {
        constexpr float DegreesPerCircle = 360.0F;
        constexpr float StraightAngle = 180.0F;
        constexpr float RadiansPerDegree = std::numbers::pi_v<float> / StraightAngle;
    }

    auto Geo::wrap360(float deg) -> float
    {
        const float wrapped = std::fmod(deg, DegreesPerCircle);
        return wrapped < 0.0F ? wrapped + DegreesPerCircle : wrapped;
    }

    auto Geo::wrap180(float deg) -> float
    {
        return wrap360(deg + StraightAngle) - StraightAngle;
    }

    auto Geo::offsetX(float angle, float distance) -> float
    {
        return std::sin(angle * RadiansPerDegree) * distance;
    }

    auto Geo::offsetY(float angle, float distance) -> float
    {
        return -std::cos(angle * RadiansPerDegree) * distance;
    }

    auto Geo::offset(float angle, float distance) -> Vec2
    {
        return Vec2{.x = offsetX(angle, distance), .y = offsetY(angle, distance)};
    }
}
