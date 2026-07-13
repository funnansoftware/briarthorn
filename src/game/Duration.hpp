#pragma once

#include <chrono>
#include <compare>

namespace bt::game
{
    /// A span of time, stored as a chrono duration — the simulation's one time
    /// type. Build it from any chrono duration (e.g. `Duration{std::chrono::
    /// milliseconds{10}}`) and read it back as seconds with toSeconds().
    class Duration
    {
    public:
        using Storage = std::chrono::duration<double>; // seconds, double precision

        Duration() noexcept = default;

        // constexpr so a Duration constant can be built at compile time; the rest
        // of the behaviour is runtime and lives in Duration.cpp.
        constexpr Duration(Storage value) noexcept : value_{value}
        {
        }

        /// The span as floating-point seconds — the form physics / dt want.
        [[nodiscard]] auto toSeconds() const -> std::chrono::duration<float>;

        auto operator+=(Duration other) noexcept -> Duration&;
        auto operator-=(Duration other) noexcept -> Duration&;
        [[nodiscard]] auto operator<=>(Duration other) const noexcept -> std::partial_ordering;

    private:
        Storage value_{};
    };
}
