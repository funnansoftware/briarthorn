#include <cstdlib>
#include <exception>
#include <iostream>

#include <game/Briarthorn.hpp>

auto main() -> int
try
{
    // Briarthorn bt;
    // bt.entities
    // bt.projection
    // bt.commands
    // bt.palette
    // bt.systems.emplace_back(Missile);
    // bt.systems.emplace_back(Radar);
    // bt.systems.emplace_back(Collision);

    // bt::Renderer renderer{bt};

    // bt.graphics = std::make_unique<Raylib>();
    // bt.graphics->addChild(std::make_unique<TrackView>());
    // bt.graphics->render();

    constexpr bt::WindowSize windowSize{.width = 800, .height = 600};

    const bt::Briarthorn bt{windowSize, "Hello, Triangle"};
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
