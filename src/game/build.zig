//! src/game — the raylib-free game core (simulation + state).
//!
//! CMake keeps two libraries (src/game, src/raylib). For the Zig build they are
//! compiled into ONE briarthorn-game archive: the emscripten and android
//! final-link helpers each take a single library, and one archive sidesteps
//! multi-archive link-order questions. Same binary, fewer moving parts. Briarthorn
//! itself lives in the app module (src/build.zig), not in this library.

const std = @import("std");
const Ctx = @import("../../zig/Ctx.zig").Ctx;
const tests = @import("test/build.zig");

/// add_library(briarthorn-game STATIC), folding the game core and the raylib edge
/// in. Links the graphics stack and returns the archive so the app (and test) can
/// link it.
pub fn configure(c: *const Ctx) *std.Build.Step.Compile {
    const mod = c.module();
    // The game core (raylib-free) + the raylib graphics edge.
    c.addSources(mod, "src/game", &.{ "Clock.cpp", "CommandBuffer.cpp", "Duration.cpp", "Entity.cpp", "Geo.cpp", "Vec2.cpp", "World.cpp", "systems/Movement.cpp" });
    c.addSources(mod, "src/raylib", &.{ "Renderer.cpp", "Window.cpp" });

    // target_link_libraries(... PUBLIC raylib) + glfw. Desktop links the graphics
    // stack here so the app and test inherit it transitively (raylib named before
    // glfw3 for the single-pass linker). Android/emscripten link raylib in their
    // platform finishApp (an object file / an emcc argument).
    if (c.isDesktop()) {
        c.linkVcpkg(mod, "raylib");
        c.linkVcpkg(mod, "glfw3");
        c.linkSystemLibs(mod);
    }

    const lib = c.b.addLibrary(.{ .name = "briarthorn-game", .linkage = .static, .root_module = mod });
    c.applyLibC(lib); // android: point at the NDK libc file; no-op elsewhere.

    // add_subdirectory(test) — desktop only, matching CMake's
    // `if(BUILD_TESTING AND NOT ANDROID AND NOT EMSCRIPTEN)`.
    if (c.isDesktop()) tests.configure(c, lib);

    return lib;
}
