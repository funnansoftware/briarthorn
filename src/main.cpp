#include <cstdlib>
#include <exception>
#include <iostream>

#include <game/Briarthorn.hpp>

auto main() -> int
try
{
    constexpr bt::WindowSize windowSize{.width = 800, .height = 600};

    bt::Briarthorn bt{windowSize, "Hello, Triangle"};

    

    bt.run();

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
