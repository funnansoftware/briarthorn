//! src — the app: an executable module that links the briarthorn library. The
//! CMakeLists.txt analog: add_subdirectory(briarthorn); add_executable (or a
//! SHARED library on android / a static library + emcc on emscripten);
//! target_sources(main.cpp); target_link_libraries(PRIVATE briarthorn).

const std = @import("std");
const config = @import("../zig/config.zig");
const Ctx = @import("../zig/Ctx.zig").Ctx;
const briarthorn = @import("briarthorn/build.zig");
const android = @import("../zig/android.zig");
const emscripten = @import("../zig/emscripten.zig");

pub fn configure(c: *const Ctx) void {
    const lib = briarthorn.configure(c); // add_subdirectory(briarthorn)

    const app = c.module();
    c.addSources(app, "src", &.{"main.cpp"});

    // What "the app" is — and how briarthorn is linked into it — depends on the
    // platform, exactly as in src/CMakeLists.txt. Each branch links briarthorn
    // once (Zig linkLibrary on desktop/android; an emcc arg on emscripten).
    switch (c.platform) {
        .windows, .linux, .macos => {
            // target_link_libraries(app PRIVATE briarthorn); raylib and the
            // system libs come transitively from briarthorn.
            app.linkLibrary(lib);
            const exe = c.b.addExecutable(.{ .name = config.app_name, .root_module = app });
            c.install(exe, "bin");
            c.runStep(exe);
        },

        .android => {
            // add_library(briarthorn-app SHARED) + NDK linking + APK packaging.
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
