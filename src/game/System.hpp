#pragma once

namespace bt::game
{
    /// @brief Abstract base class for all game systems.
    class System
    {
    public:
        System() = default;
        virtual ~System() = default;
        System(const System&) = delete;
        auto operator=(const System&) -> System& = delete;
        System(System&&) noexcept = delete;
        auto operator=(System&&) noexcept -> System& = delete;

        virtual auto update(/*entities*/ float x) -> void = 0;
    };
}