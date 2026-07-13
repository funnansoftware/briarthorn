#include <game/Duration.hpp>

#include <chrono>
#include <compare>

namespace bt::game
{
    auto Duration::toSeconds() const -> std::chrono::duration<float>
    {
        return value_;
    }

    auto Duration::operator+=(Duration other) noexcept -> Duration&
    {
        value_ += other.value_;
        return *this;
    }

    auto Duration::operator-=(Duration other) noexcept -> Duration&
    {
        value_ -= other.value_;
        return *this;
    }

    auto Duration::operator<=>(Duration other) const noexcept -> std::partial_ordering
    {
        return value_ <=> other.value_;
    }
}
