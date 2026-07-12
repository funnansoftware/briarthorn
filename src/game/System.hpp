#pragma once

namespace bt
{
    /// @brief Abstract base class for all game systems.
    class System
    {
    public:
        virtual ~System() = default;
        System(const System&) = delete;
        auto operator=(const System&) -> System& = delete;
        System(System&&) noexcept = delete;
        auto operator=(System&&) noexcept -> System& = delete;

        virtual auto update(/*entities*/ float x) -> void = 0;
    };
}