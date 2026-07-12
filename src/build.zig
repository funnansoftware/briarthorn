//! src — the app: an executable module that links the briarthorn-game library.
//! The CMakeLists.txt analog: add_subdirectory(game); add_executable (or a
//! SHARED library on android / a static library + emcc on emscripten);
//! target_sources(main.cpp); target_link_libraries(PRIVATE briarthorn-game).

const std = @import("std");
const config = @import("../zig/config.zig");
const Ctx = @import("../zig/Ctx.zig").Ctx;
const game = @import("game/build.zig");
const android = @import("../zig/android.zig");
const emscripten = @import("../zig/emscripten.zig");

pub fn configure(c: *const Ctx) void {
    const lib = game.configure(c); // add_subdirectory(game)

    const app = c.module();
    c.addSources(app, "src", &.{"main.cpp"});

    // What "the app" is — and how briarthorn-game is linked into it — depends on
    // the platform, exactly as in src/CMakeLists.txt. Each branch links the
    // library once (Zig linkLibrary on desktop/android; an emcc arg on emscripten).
    switch (c.platform) {
        .windows, .linux, .macos => {
            // target_link_libraries(app PRIVATE briarthorn-game); raylib and the
            // system libs come transitively from briarthorn-game.
            app.linkLibrary(lib);
            const exe = c.b.addExecutable(.{ .name = config.app_name, .root_module = app });
            c.install(exe, "bin");
            c.runStep(exe);
        },

        .android => {
            // add_library(briarthorn SHARED) + NDK linking + APK packaging.
            android.finishApp(c, app, lib);
            c.stubDesktopOnlySteps();
        },
        .emscripten => {
            // static library + emcc -> .html/.js/.wasm.
            emscripten.finishApp(c, app, lib);
            c.stubDesktopOnlySteps();
        },
    }
}
