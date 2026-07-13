#include <game/Entity.hpp>

namespace bt::game
{
    auto Entity::topSpeed() const -> float
    {
        return topSpeedBase * speedBoost;
    }
}
