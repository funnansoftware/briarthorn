#pragma once

#include <chrono>

#include <game/Duration.hpp>

namespace bt::game
{
    // The default catch-up cap: at most this many fixed steps run for one real
    // frame, so a stall can't spiral into ever-growing catch-up.
    inline constexpr int DefaultMaxSteps = 8;

    /// A fixed-timestep clock built on a monotonic chrono source. Real elapsed
    /// time is accumulated (as a Duration) and released in whole fixed steps, so
    /// the simulation always advances by an identical step regardless of render
    /// framerate — the determinism a command-buffer sim (and later replay /
    /// lockstep netcode) wants. This owns the timekeeping.
    class Clock
    {
    public:
        using Source = std::chrono::steady_clock; // monotonic: never jumps back

        /// [fixedStep] is the tick length (e.g. 10 ms). [maxSteps] caps how many
        /// ticks one frame may run, so a stall can't spiral into ever-growing
        /// catch-up (the "spiral of death" guard).
        explicit Clock(Duration fixedStep, int maxSteps = DefaultMaxSteps);

        /// Redefine the fixed step (the sim rate). Banked time is kept, so it takes
        /// effect from the next tick.
        auto setInterval(Duration fixedStep) -> void;

        [[nodiscard]] auto interval() const -> Duration;

        /// Re-stamp "now" and drop any banked time — call it the moment real-time
        /// stepping begins (e.g. after the window opens), so slow start-up work is
        /// not folded into the first frame as a catch-up burst.
        auto reset() -> void;

        /// Sample the monotonic clock, fold the elapsed real time into the
        /// accumulator, and return how many fixed steps are now due (0..maxSteps).
        auto tick() -> int;

        /// Fold an explicit elapsed duration in — the testable core of tick(),
        /// and the seam for driving the clock from a fixed source in tests.
        auto advance(Duration elapsed) -> int;

        /// The fixed step, in seconds, to hand each system's step(world, dt).
        [[nodiscard]] auto fixedSeconds() const -> float;

        /// How far (0..1) into the next step the accumulator sits — the blend
        /// factor a renderer can use to interpolate between the last two states.
        [[nodiscard]] auto alpha() const -> float;

    private:
        Duration fixedStep_;
        int maxSteps_;
        Duration accumulator_;
        Source::time_point last_;
    };
}
