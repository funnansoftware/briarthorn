#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <game/Clock.hpp>
#include <game/CommandBuffer.hpp>
#include <game/Duration.hpp>
#include <game/System.hpp>
#include <game/World.hpp>

#include <raylib/Renderer.hpp>

namespace bt
{
    // The default fixed simulation step: 10 ms (i.e. 100 steps per second).
    inline constexpr auto DefaultStepInterval = game::Duration{std::chrono::milliseconds{10}};

    /// @brief The owning object of the app and its state: the world, the ordered
    /// systems, the fixed-timestep clock and the command buffer.
    ///
    /// It runs headless — the simulation steps with no graphics at all, which is how
    /// tests and any future server drive it — or drives an interactive loop once
    /// graphics are enabled.
    ///
    /// @code
    /// bt::Briarthorn briarthorn;
    /// briarthorn.initGraphics();   // opens the window; omit to stay headless
    /// briarthorn.run();
    /// @endcode
    class Briarthorn
    {
    public:
        explicit Briarthorn();

        /// @brief Set the fixed simulation step interval (e.g.
        /// `game::Duration{std::chrono::milliseconds{5}}`).
        ///
        /// Takes effect on the next step().
        /// @param step The fixed simulation step interval to set.
        auto setStepInterval(game::Duration step) -> void;

        /// @brief Enable graphics: open a @p width x @p height window titled @p title
        /// and hold the raylib surface.
        ///
        /// Until this is called the app is headless. Calling it again re-opens the
        /// window.
        /// @param title The window title.
        /// @param width The window width in pixels.
        /// @param height The window height in pixels.
        auto initGraphics(std::string title = "briarthorn", int width = raylib::DefaultWindowWidth, int height = raylib::DefaultWindowHeight) -> void;

        /// @brief Accumulate the real time elapsed (via the chrono clock) since the
        /// last call and run every fixed step now due: for each, flush the command
        /// buffer then step every system.
        ///
        /// Zero, one, or several updates may run, so the sim holds its fixed rate no
        /// matter how often update() is called. No graphics needed — the headless entry
        /// point.
        auto update() -> void;

        /// @brief Drive the loop until stopped: each iteration advances the
        /// simulation by the fixed updates now due (update(), on its own clock) and —
        /// when graphics are enabled — polls input before and renders after.
        ///
        /// Runs headless too, with no window and no render. Ends when the window
        /// closes or stop() is called, and is safe to call again afterwards.
        auto run() -> void;

        /// @brief Ask run()'s loop to finish after the current iteration.
        ///
        /// The window's close button does this on its own; call it to end a headless
        /// run.
        auto stop() -> void;

        [[nodiscard]] auto world() -> game::World&;
        [[nodiscard]] auto world() const -> const game::World&;
        [[nodiscard]] auto commands() -> game::CommandBuffer&;

    private:
        auto buildWorld() -> void;

        game::World world_;
        game::CommandBuffer commands_;
        std::vector<std::unique_ptr<game::System>> systems_;
        game::Clock clock_;
        std::optional<raylib::Renderer> graphics_;
        bool running_{false};
    };
}
