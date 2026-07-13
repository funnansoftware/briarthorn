#include <cstdlib>
#include <exception>
#include <iostream>

#include <Briarthorn.hpp>

auto main() -> int
try
{
    auto briarthorn = bt::Briarthorn{};

    // Enable graphics (opens the window). Drop this line and the same Briarthorn
    // runs headless.
    briarthorn.initGraphics();

    briarthorn.run();

    return EXIT_SUCCESS;
}
catch (const std::exception& error)
{
    std::cerr << "briarthorn: " << error.what() << '\n';
    return EXIT_FAILURE;
}
catch (...)
{
    std::cerr << "briarthorn: unknown error\n";
    return EXIT_FAILURE;
}
