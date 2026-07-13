#include <game/Clock.hpp>

namespace bt::game
{
    Clock::Clock(Duration fixedStep, int maxSteps) : fixedStep_{fixedStep}, maxSteps_{maxSteps}, last_{Source::now()}
    {
    }

    auto Clock::setInterval(Duration fixedStep) -> void
    {
        fixedStep_ = fixedStep;
    }

    auto Clock::interval() const -> Duration
    {
        return fixedStep_;
    }

    auto Clock::reset() -> void
    {
        last_ = Source::now();
        accumulator_ = Duration{};
    }

    auto Clock::tick() -> int
    {
        const auto now = Source::now();
        const Duration elapsed{now - last_};
        last_ = now;
        return advance(elapsed);
    }

    auto Clock::advance(Duration elapsed) -> int
    {
        accumulator_ += elapsed;

        int steps = 0;
        while (accumulator_ >= fixedStep_ && steps < maxSteps_)
        {
            accumulator_ -= fixedStep_;
            ++steps;
        }

        // Drop any backlog beyond the cap so we don't perpetually chase it.
        if (accumulator_ >= fixedStep_)
        {
            accumulator_ = Duration{};
        }

        return steps;
    }

    auto Clock::fixedSeconds() const -> float
    {
        return fixedStep_.toSeconds().count();
    }

    auto Clock::alpha() const -> float
    {
        return accumulator_.toSeconds() / fixedStep_.toSeconds();
    }
}
