//! src/briarthorn/test — an executable module that links the briarthorn library.
//! The CMakeLists.txt analog: add_executable(briarthorn-test) +
//! target_link_libraries(briarthorn-test PRIVATE briarthorn GTest::gtest_main).

const std = @import("std");
const config = @import("../../../zig/config.zig");
const Ctx = @import("../../../zig/Ctx.zig").Ctx;

/// Builds the GoogleTest executable that links `briarthorn` (rather than
/// recompiling it) and wires the `test` step. Desktop only.
pub fn configure(c: *const Ctx, briarthorn: *std.Build.Step.Compile) void {
    const mod = c.module();
    // Only the test translation units: main() comes from gtest_main, and the
    // library code comes from linking briarthorn.
    c.addSources(mod, "src/briarthorn/test", &.{"Game.test.cpp"});

    // target_link_libraries(briarthorn-test PRIVATE briarthorn GTest::gtest_main).
    // raylib/glfw/system libs are inherited from briarthorn; the test only names
    // its own vcpkg dep (gtest_main before gtest for the single-pass linker).
    mod.linkLibrary(briarthorn);
    c.linkVcpkg(mod, "gtest_main");
    c.linkVcpkg(mod, "gtest");

    const exe = c.b.addExecutable(.{ .name = config.test_name, .root_module = mod });
    c.testStep(exe);
}
