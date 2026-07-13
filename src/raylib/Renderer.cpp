#include <raylib/Renderer.hpp>

#include <raylib.h>

#include <game/CommandBuffer.hpp>
#include <game/Entity.hpp>
#include <game/Geo.hpp>
#include <game/Vec2.hpp>
#include <game/World.hpp>

namespace bt::raylib
{
    namespace
    {
        // Marker geometry (metres): a small triangle pointing along the heading.
        constexpr float MarkerNose = 12.0F;
        constexpr float MarkerTail = 9.0F;
        constexpr float MarkerTailAngle = 140.0F; // heading offset to each tail corner (deg)
        constexpr float Half = 0.5F;

        [[nodiscard]] auto toScreen(bt::game::Vec2 world, bt::game::Vec2 camera, Vector2 center) -> Vector2
        {
            return Vector2{.x = center.x + (world.x - camera.x), .y = center.y + (world.y - camera.y)};
        }

        auto drawMarker(const bt::game::Entity& entity, bt::game::Vec2 camera, Vector2 center, Color color) -> void
        {
            const bt::game::Vec2 nose = entity.position + bt::game::Geo::offset(entity.heading, MarkerNose);
            const bt::game::Vec2 tailLeft = entity.position + bt::game::Geo::offset(entity.heading - MarkerTailAngle, MarkerTail);
            const bt::game::Vec2 tailRight = entity.position + bt::game::Geo::offset(entity.heading + MarkerTailAngle, MarkerTail);

            // Vertex order (nose, left, right) matches the fill winding raylib used
            // for the original hello-triangle in this screen's y-down space.
            DrawTriangle(toScreen(nose, camera, center), toScreen(tailLeft, camera, center), toScreen(tailRight, camera, center), color);
        }
    }

    Renderer::Renderer() : Renderer(Config{})
    {
    }

    Renderer::Renderer(const Config& config) : window_{{.title = config.title, .width = config.width, .height = config.height}}
    {
    }

    auto Renderer::shouldClose() const -> bool
    {
        return WindowShouldClose();
    }

    auto Renderer::pollInput(bt::game::CommandBuffer& commands, bt::game::EntityId player) -> void
    {
        if (player == bt::game::NullEntity)
        {
            return;
        }

        const bool forward = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
        const bool backward = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
        const bool left = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
        const bool right = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
        const bool boost = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        commands.throttle(player, forward ? 1.0F : 0.0F);
        commands.brake(player, backward ? 1.0F : 0.0F);
        commands.steer(player, (right ? 1.0F : 0.0F) - (left ? 1.0F : 0.0F));
        commands.boost(player, boost ? 1.0F : 0.0F);
    }

    auto Renderer::render(const bt::game::World& world) -> void
    {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        const Vector2 center{.x = static_cast<float>(GetScreenWidth()) * Half, .y = static_cast<float>(GetScreenHeight()) * Half};

        // Ownship-centred camera: the player (if any) sits at screen centre.
        bt::game::Vec2 camera{};
        if (const bt::game::Entity* found = world.find(world.player()); found != nullptr)
        {
            camera = found->position;
        }

        for (const auto& entity : world.entities())
        {
            const Color color = (entity.id == world.player()) ? ORANGE : SKYBLUE;
            drawMarker(entity, camera, center, color);
        }

        EndDrawing();
    }
}
