#include <raylib/Renderer.hpp>

#include <raylib.h>

#include <game/CommandBuffer.hpp>
#include <game/Entity.hpp>
#include <game/Geo.hpp>
#include <game/Vec2.hpp>
#include <game/World.hpp>

using bt::raylib::Renderer;

namespace
{
    // Marker geometry (metres): a small triangle pointing along the heading.
    constexpr auto MarkerNose = 12.0F;
    constexpr auto MarkerTail = 9.0F;
    constexpr auto MarkerTailAngle = 140.0F; // heading offset to each tail corner (deg)
    constexpr auto Half = 0.5F;

    [[nodiscard]] auto toScreen(bt::game::Vec2 world, bt::game::Vec2 camera, Vector2 center) -> Vector2
    {
        return Vector2{.x = center.x + (world.x - camera.x), .y = center.y + (world.y - camera.y)};
    }

    auto drawMarker(const bt::game::Entity& entity, bt::game::Vec2 camera, Vector2 center, Color color) -> void
    {
        const auto nose = entity.position + bt::game::Geo::offset(entity.heading, MarkerNose);
        const auto tailLeft = entity.position + bt::game::Geo::offset(entity.heading - MarkerTailAngle, MarkerTail);
        const auto tailRight = entity.position + bt::game::Geo::offset(entity.heading + MarkerTailAngle, MarkerTail);

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

    const auto forward = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
    const auto backward = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);
    const auto left = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
    const auto right = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
    const auto boost = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    commands.throttle(player, forward ? 1.0F : 0.0F);
    commands.brake(player, backward ? 1.0F : 0.0F);
    commands.steer(player, (right ? 1.0F : 0.0F) - (left ? 1.0F : 0.0F));
    commands.boost(player, boost ? 1.0F : 0.0F);
}

auto Renderer::render(const bt::game::World& world) -> void
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    const auto center = Vector2{.x = static_cast<float>(GetScreenWidth()) * Half, .y = static_cast<float>(GetScreenHeight()) * Half};

    // Ownship-centred camera: the player (if any) sits at screen centre.
    auto camera = bt::game::Vec2{};
    if (const auto* found = world.find(world.getPlayer()); found != nullptr)
    {
        camera = found->position;
    }

    for (const auto& entity : world.entities())
    {
        const auto color = (entity.id == world.getPlayer()) ? ORANGE : SKYBLUE;
        drawMarker(entity, camera, center, color);
    }

    EndDrawing();
    PollInputEvents();
    SwapScreenBuffer();
}
