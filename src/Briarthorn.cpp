#include <Briarthorn.hpp>

#include <memory>
#include <utility>

#include <game/Duration.hpp>
#include <game/Entity.hpp>
#include <game/Vec2.hpp>
#include <game/systems/Movement.hpp>
#include <raylib/Renderer.hpp>

namespace bt
{
    namespace
    {
        // Demo scaffolding: two static reference markers around the origin (metres).
        constexpr float MarkerNorthX = 120.0F;
        constexpr float MarkerNorthY = -80.0F;
        constexpr float MarkerSouthX = -160.0F;
        constexpr float MarkerSouthY = 120.0F;
    }

    Briarthorn::Briarthorn(game::Duration step) : clock_{step}
    {
        buildWorld();
        systems_.push_back(std::make_unique<game::Movement>());
    }

    auto Briarthorn::setStepInterval(game::Duration step) -> void
    {
        clock_.setInterval(step);
    }

    auto Briarthorn::initGraphics(std::string title, int width, int height) -> void
    {
        graphics_.emplace(raylib::Renderer::Config{.title = std::move(title), .width = width, .height = height});
    }

    auto Briarthorn::buildWorld() -> void
    {
        // Ownship at the origin, plus a couple of static markers so the
        // ownship-centred camera has fixed references to fly against.
        const game::Entity ownship{};
        world_.setPlayer(world_.spawn(ownship));

        const game::Entity north{.position = game::Vec2{.x = MarkerNorthX, .y = MarkerNorthY}};
        world_.spawn(north);

        const game::Entity south{.position = game::Vec2{.x = MarkerSouthX, .y = MarkerSouthY}};
        world_.spawn(south);
    }

    auto Briarthorn::step() -> void
    {
        // Fold the real time elapsed since the last call into the chrono clock; it
        // returns how many fixed steps are now due (0..maxSteps).
        const int steps = clock_.tick();
        const float dt = clock_.fixedSeconds();
        for (int i = 0; i < steps; ++i)
        {
            // Controller tier first (apply this tick's recorded intent), then the
            // authoritative systems integrate it.
            commands_.flush(world_);
            for (const auto& system : systems_)
            {
                system->step(world_, dt);
            }
        }
    }

    auto Briarthorn::world() -> game::World&
    {
        return world_;
    }

    auto Briarthorn::world() const -> const game::World&
    {
        return world_;
    }

    auto Briarthorn::commands() -> game::CommandBuffer&
    {
        return commands_;
    }

    auto Briarthorn::run() -> void
    {
        if (!graphics_)
        {
            return; // headless: there is no window loop to drive
        }

        clock_.reset(); // start real-time stepping now, after the window has opened
        while (!graphics_->shouldClose())
        {
            // Sample device input once per frame; step() flushes and consumes it at
            // the next fixed tick boundary.
            graphics_->pollInput(commands_, world_.player());
            step();
            graphics_->render(world_);
        }
    }
}
