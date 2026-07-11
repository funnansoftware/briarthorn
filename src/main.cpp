#include <cstdlib>

#include <briarthorn/Game.hpp>

auto main() -> int
try
{
    constexpr briarthorn::WindowSize windowSize{.width = 800, .height = 600};

    const briarthorn::Game game{windowSize, "Hello, Triangle"};
    game.run();

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
