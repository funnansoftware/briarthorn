#include <cstdlib>
#include <exception>
#include <iostream>

#include <briarthorn/Game.hpp>

auto main() -> int
try
{
    constexpr briarthorn::WindowSize windowSize{.width = 800, .height = 600};

    const briarthorn::Game game{windowSize, "Hello, Triangle"};
    game.run();

    return EXIT_SUCCESS;
}
catch (const std::exception &error)
{
    std::cerr << "briarthorn: " << error.what() << '\n';
    return EXIT_FAILURE;
}
catch (...)
{
    std::cerr << "briarthorn: unknown error\n";
    return EXIT_FAILURE;
}
