#include <game/Vec2.hpp>

#include <cmath>

namespace bt::game
{
    auto Vec2::lengthSquared() const -> float
    {
        return (x * x) + (y * y);
    }

    auto Vec2::length() const -> float
    {
        return std::sqrt(lengthSquared());
    }

    auto operator+(Vec2 a, Vec2 b) -> Vec2
    {
        return Vec2{.x = a.x + b.x, .y = a.y + b.y};
    }

    auto operator-(Vec2 a, Vec2 b) -> Vec2
    {
        return Vec2{.x = a.x - b.x, .y = a.y - b.y};
    }

    auto operator*(Vec2 v, float scale) -> Vec2
    {
        return Vec2{.x = v.x * scale, .y = v.y * scale};
    }
}
