//! src/game — a linkable library module. The CMakeLists.txt analog:
//! add_library(briarthorn-game STATIC) + target_sources + add_subdirectory(test).

const std = @import("std");
const Ctx = @import("../../zig/Ctx.zig").Ctx;
const tests = @import("test/build.zig");

/// add_library(briarthorn-game STATIC). Returns the archive so the app (and the
/// test) can link it (target_link_libraries).
pub fn configure(c: *const Ctx) *std.Build.Step.Compile {
    const mod = c.module();
    c.addSources(mod, "src/game", &.{"Briarthorn.cpp"});

    // target_link_libraries(briarthorn-game PUBLIC raylib) + glfw. Desktop links
    // the graphics stack here so the app and test inherit it transitively (raylib
    // named before glfw3 for the single-pass linker). Android/emscripten link
    // raylib in their platform finishApp (an object file / an emcc argument).
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
