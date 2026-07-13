#pragma once

#include <string>

#include <game/Entity.hpp>
#include <game/Graphics.hpp>

#include <raylib/Window.hpp>

namespace bt::game
{
    class World;
    class CommandBuffer;
}

namespace bt::raylib
{
    inline constexpr int DefaultWindowWidth = 800;
    inline constexpr int DefaultWindowHeight = 600;

    /// The raylib graphics surface attached to a Briarthorn. It owns the window
    /// and implements bt::game::Graphics: it presents the world (ownship-centred,
    /// 1 px = 1 m) and reads the keyboard into the command buffer. The concrete
    /// `graphics` a headless Briarthorn does without.
    class Renderer : public bt::game::Graphics
    {
    public:
        struct Config
        {
            std::string title = "briarthorn";
            int width = DefaultWindowWidth;
            int height = DefaultWindowHeight;
        };

        Renderer();
        explicit Renderer(const Config& config);

        [[nodiscard]] auto shouldClose() const -> bool override;
        auto pollInput(bt::game::CommandBuffer& commands, bt::game::EntityId player) -> void override;
        auto render(const bt::game::World& world) -> void override;

    private:
        Window window_;
    };
}
