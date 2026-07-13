#include <game/Duration.hpp>

#include <chrono>
#include <compare>

namespace bt::game
{
    auto Duration::toSeconds() const -> std::chrono::duration<float>
    {
        return duration_;
    }

    auto Duration::operator+=(Duration other) noexcept -> Duration&
    {
        duration_ += other.duration_;
        return *this;
    }

    auto Duration::operator-=(Duration other) noexcept -> Duration&
    {
        duration_ -= other.duration_;
        return *this;
    }

    auto Duration::operator<=>(Duration other) const noexcept -> std::partial_ordering
    {
        return duration_ <=> other.duration_;
    }
}
