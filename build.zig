const std = @import("std");

pub fn build(b: *std.Build) void {
    // Target and optimization setups
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe_mod = b.addModule("exe", .{
        .target = target,
        .optimize = optimize,
        .link_libcpp = true, // May need to change this to linkLibC() for your project
    });

    exe_mod.addCSourceFiles(.{
        .files = &.{
            "app/hello-triangle/main.cpp",
        },
        .flags = &.{
            "-std=c++23",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-pedantic",
        },
    });

    exe_mod.addIncludePath(b.graph.cwdRelativePath("build/x64-linux-zig-debug/vcpkg_installed/x64-linux/include"));
    exe_mod.addLibraryPath(b.graph.cwdRelativePath("build/x64-linux-zig-debug/vcpkg_installed/x64-linux/lib"));

    // exe_mod.addIncludePath(
    //     b.graph.cwdRelativePath("build/x64-windows-zig-debug/vcpkg_installed/x64-mingw-static/include"),
    // );
    // exe_mod.addLibraryPath(
    //     b.graph.cwdRelativePath("build/x64-windows-zig-debug/vcpkg_installed/x64-mingw-static/lib"),
    // );
    exe_mod.linkSystemLibrary("raylib", .{});
    exe_mod.linkSystemLibrary("glfw3", .{});

    // X11 libraries required by raylib on Linux
    exe_mod.linkSystemLibrary("X11", .{});
    exe_mod.linkSystemLibrary("Xrandr", .{});
    exe_mod.linkSystemLibrary("Xinerama", .{});
    exe_mod.linkSystemLibrary("Xi", .{});
    exe_mod.linkSystemLibrary("Xcursor", .{});
    exe_mod.linkSystemLibrary("Xfixes", .{});

    // exe_mod.linkSystemLibrary("gdi32", .{});

    const exe = b.addExecutable(.{
        .name = "hello-triangle",
        .root_module = exe_mod,
    });

    // Include directory paths
    // exe.addIncludePath(b.path("include"));

    // Install artifact
    b.installArtifact(exe);

    // Create a run step (zig build run)
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
