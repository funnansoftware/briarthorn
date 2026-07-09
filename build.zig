const std = @import("std");

const app_name = "hello-triangle";
const app_sources = [_][]const u8{"app/hello-triangle/main.cpp"};
const cxx_flags = [_][]const u8{ "-std=c++23", "-Wall", "-Wextra", "-Werror", "-pedantic" };

const Platform = enum { windows, linux, macos, android, emscripten };

pub fn build(b: *std.Build) void {
    var target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const platform = classify(target.result);

    // Android needs a concrete API level to pick bionic CRT objects; fold it
    // into the target so __ANDROID_API__ matches what we link against.
    const android_api = b.option(u32, "android-api", "Android API level (android targets only)") orelse 35;
    if (platform == .android and target.query.android_api_level == null) {
        var query = target.query;
        query.android_api_level = android_api;
        target = b.resolveTargetQuery(query);
    }

    const triplet_override = b.option([]const u8, "vcpkg-triplet", "Override the vcpkg target triplet");
    if (triplet_override) |t|
        std.debug.print("note: vcpkg triplet overridden to {s}; you are responsible for ABI compatibility\n", .{t});
    const triplet = triplet_override orelse vcpkgTriplet(platform, target.result);
    const host_triplet = vcpkgHostTriplet(b);

    // Mirror the CMake preset layout: everything for one target lives under
    // build/<target>/ — vcpkg packages in vcpkg_installed/, final binaries in
    // installed/. vcpkg manifest mode syncs an install root to a single
    // triplet and evicts any other triplet's packages, so the per-target root
    // also keeps switching between `zig build` targets cheap.
    const target_dir = b.fmt("build/{s}", .{targetDirName(b, platform, target.result)});
    const vcpkg_root = b.fmt("{s}/vcpkg_installed", .{target_dir});
    const install_prefix = b.fmt("{s}/installed", .{target_dir});

    ensureVcpkgInstalled(b, platform, triplet, host_triplet, vcpkg_root);

    const installed = b.fmt("{s}/{s}", .{ vcpkg_root, triplet });
    const include_rel = b.fmt("{s}/include", .{installed});
    const lib_rel = if (optimize == .Debug)
        b.fmt("{s}/debug/lib", .{installed})
    else
        b.fmt("{s}/lib", .{installed});

    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        // Zig's bundled libc++ cannot be used with an Android libc file
        // (ziglang/zig#23302); the android branch links the NDK's libc++ instead.
        .link_libcpp = if (platform == .android) null else true,
        .link_libc = if (platform == .android) true else null,
    });
    mod.addCSourceFiles(.{ .files = &app_sources, .flags = &cxx_flags });
    mod.addSystemIncludePath(b.path(include_rel));

    switch (platform) {
        .windows, .linux, .macos => buildDesktop(b, mod, platform, lib_rel, install_prefix),
        .android, .emscripten => {
            // The SDK env vars (ANDROID_NDK_HOME, EMSDK) and directory scans
            // below are host observations the configure cache cannot track; a
            // cached configuration would bake stale absolute SDK paths.
            b.graph.poisonCache();
            if (platform == .android)
                buildAndroid(b, mod, target, optimize, android_api, lib_rel, target_dir, install_prefix)
            else
                buildEmscripten(b, mod, optimize, lib_rel, install_prefix);

            const run_step = b.step("run", "Run the app");
            run_step.dependOn(&b.addFail("the run step is only available for desktop targets").step);
        },
    }
}

/// Directory name under build/ for a target, mirroring the CMake preset
/// layout: the resolved arch-os-abi triple (e.g. build/aarch64-linux-android).
fn targetDirName(b: *std.Build, platform: Platform, t: std.Target) []const u8 {
    if (platform == .emscripten) return b.fmt("{s}-emscripten", .{@tagName(t.cpu.arch)});
    if (t.abi == .none) return b.fmt("{s}-{s}", .{ @tagName(t.cpu.arch), @tagName(t.os.tag) });
    return b.fmt("{s}-{s}-{s}", .{ @tagName(t.cpu.arch), @tagName(t.os.tag), @tagName(t.abi) });
}

/// The install prefix is fixed to build/<target>/installed. Zig's own install
/// steps can only target the maker-side --prefix (default zig-out), so
/// installs are modeled as build-root-relative copies instead.
fn installFile(
    b: *std.Build,
    usf: *std.Build.Step.UpdateSourceFiles,
    src: std.Build.LazyPath,
    install_prefix: []const u8,
    sub_dir: []const u8,
    basename: []const u8,
) void {
    usf.addCopyFileToSource(src, b.fmt("{s}/{s}/{s}", .{ install_prefix, sub_dir, basename }));
}

// -- Desktop (windows/linux/macos) -------------------------------------------

fn buildDesktop(
    b: *std.Build,
    mod: *std.Build.Module,
    platform: Platform,
    lib_rel: []const u8,
    install_prefix: []const u8,
) void {
    const vcpkg_lib: std.Build.Module.LinkSystemLibraryOptions = .{
        .use_pkg_config = .no,
        .preferred_link_mode = .static,
    };

    mod.addLibraryPath(b.path(lib_rel));
    mod.linkSystemLibrary("raylib", vcpkg_lib);
    // glfw after raylib so the linker can resolve raylib's references to glfw
    // symbols (single-pass left-to-right resolution).
    mod.linkSystemLibrary("glfw3", vcpkg_lib);

    switch (platform) {
        .windows => {
            const win_libs = [_][]const u8{
                "user32", "gdi32",    "winspool", "shell32",  "ole32",
                "uuid",   "oleaut32", "comdlg32", "advapi32",
            };
            for (win_libs) |name| mod.linkSystemLibrary(name, .{ .use_pkg_config = .no });
        },
        .linux => {
            const x11_libs = [_][]const u8{ "X11", "Xrandr", "Xinerama", "Xi", "Xcursor", "Xfixes" };
            for (x11_libs) |name| mod.linkSystemLibrary(name, .{});
        },
        .macos => {
            const frameworks = [_][]const u8{
                "OpenGL", "Cocoa", "IOKit", "CoreFoundation", "CoreVideo", "CoreAudio",
            };
            for (frameworks) |name| mod.linkFramework(name, .{});
        },
        else => unreachable,
    }

    const exe = b.addExecutable(.{
        .name = app_name,
        .root_module = mod,
    });

    const install_files = b.addUpdateSourceFiles();
    installFile(b, install_files, exe.getEmittedBin(), install_prefix, "bin", exe.out_filename);
    if (platform == .windows)
        installFile(b, install_files, exe.getEmittedPdb(), install_prefix, "bin", app_name ++ ".pdb");
    b.getInstallStep().dependOn(&install_files.step);

    const run_step = b.step("run", "Run the app");
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    run_cmd.addPassthruArgs();
    run_step.dependOn(&run_cmd.step);
}

// -- Android ------------------------------------------------------------------

fn buildAndroid(
    b: *std.Build,
    mod: *std.Build.Module,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    default_api: u32,
    lib_rel: []const u8,
    target_dir: []const u8,
    install_prefix: []const u8,
) void {
    const ndk = b.graph.environ_map.get("ANDROID_NDK_HOME") orelse
        std.process.fatal("ANDROID_NDK_HOME must point at an Android NDK for android targets", .{});
    const host_tag: []const u8 = switch (b.graph.host.result.os.tag) {
        .windows => "windows-x86_64",
        .linux => "linux-x86_64",
        .macos => "darwin-x86_64",
        else => std.process.fatal("unsupported host for android builds", .{}),
    };
    const arch_name: []const u8 = switch (target.result.cpu.arch) {
        .aarch64 => "aarch64-linux-android",
        .x86_64 => "x86_64-linux-android",
        else => unreachable, // filtered by vcpkgTriplet
    };
    const api = target.query.android_api_level orelse default_api;

    const prebuilt = b.pathJoin(&.{ ndk, "toolchains", "llvm", "prebuilt", host_tag });
    const sys_include = b.pathJoin(&.{ prebuilt, "sysroot", "usr", "include" });
    const lib_unversioned = b.pathJoin(&.{ prebuilt, "sysroot", "usr", "lib", arch_name });
    const crt_dir = b.pathJoin(&.{ lib_unversioned, b.fmt("{d}", .{api}) });

    // Zig does not ship bionic; describe the NDK's libc with a libc file.
    const write_files = b.addWriteFiles();
    const libc_file = write_files.add("android-libc.txt", b.fmt(
        \\include_dir={s}
        \\sys_include_dir={s}
        \\crt_dir={s}
        \\msvc_lib_dir=
        \\kernel32_lib_dir=
        \\gcc_dir=
        \\
    , .{ sys_include, b.pathJoin(&.{ sys_include, arch_name }), crt_dir }));

    // NDK libc++ headers replace zig's bundled ones (see comment at module creation).
    mod.addIncludePath(b.graph.cwdRelativePath(b.pathJoin(&.{ sys_include, "c++", "v1" })));

    mod.addLibraryPath(b.graph.cwdRelativePath(crt_dir));
    mod.addLibraryPath(b.graph.cwdRelativePath(lib_unversioned));

    // raylib is a NativeActivity app: it provides ANativeActivity_onCreate and
    // android_main, and calls the app's regular main().
    mod.addObjectFile(b.path(b.pathJoin(&.{ lib_rel, "libraylib.a" })));
    mod.addObjectFile(b.graph.cwdRelativePath(b.pathJoin(&.{ lib_unversioned, "libc++_static.a" })));
    mod.addObjectFile(b.graph.cwdRelativePath(b.pathJoin(&.{ lib_unversioned, "libc++abi.a" })));
    if (clangRuntimeLibDir(b, prebuilt, target.result.cpu.arch)) |dir| {
        mod.addLibraryPath(b.graph.cwdRelativePath(dir));
        mod.linkSystemLibrary("unwind", .{ .use_pkg_config = .no });
    }
    const android_libs = [_][]const u8{ "log", "android", "GLESv2", "EGL", "OpenSLES" };
    for (android_libs) |name| mod.linkSystemLibrary(name, .{ .use_pkg_config = .no });

    const lib = b.addLibrary(.{
        .linkage = .dynamic,
        .name = app_name,
        .root_module = mod,
    });
    lib.setLibCFile(libc_file);
    libc_file.addStepDependencies(&lib.step);
    // NativeActivity looks the entry point up in the shared library; force it
    // to be pulled out of the static raylib archive.
    lib.forceUndefinedSymbol("ANativeActivity_onCreate");
    lib.use_llvm = true;
    lib.use_lld = true;
    // Provides the __aarch64_* outline atomics the NDK expects from compiler-rt.
    lib.bundle_compiler_rt = true;
    // Android 15+ requires 16 KiB page alignment.
    lib.link_z_max_page_size = 16384;

    const install_files = b.addUpdateSourceFiles();
    installFile(b, install_files, lib.getEmittedBin(), install_prefix, "lib", lib.out_filename);
    b.getInstallStep().dependOn(&install_files.step);

    // Package the library into an APK with the gradle project in android/.
    // The -PzigJniLibs property makes gradle package the staged zig-built
    // library instead of running its own CMake externalNativeBuild.
    ensureAndroidSdkConfigured(b, ndk);
    const abi: []const u8 = switch (target.result.cpu.arch) {
        .aarch64 => "arm64-v8a",
        .x86_64 => "x86_64",
        else => unreachable, // filtered by vcpkgTriplet
    };
    const jni_dir = b.fmt("{s}/jniLibs", .{target_dir});
    const jni_stage = b.addUpdateSourceFiles();
    jni_stage.addCopyFileToSource(
        lib.getEmittedBin(),
        b.fmt("{s}/{s}/{s}", .{ jni_dir, abi, lib.out_filename }),
    );

    const root = rootPath(b);
    const host_is_windows = b.graph.host.result.os.tag == .windows;
    const gradlew = b.pathJoin(&.{ root, "android", if (host_is_windows) "gradlew.bat" else "gradlew" });
    const gradle = if (host_is_windows)
        b.addSystemCommand(&.{gradlew})
    else
        b.addSystemCommand(&.{ "sh", gradlew });
    gradle.addArgs(&.{ "-p", b.pathJoin(&.{ root, "android" }) });
    gradle.addArg(if (optimize == .Debug) "assembleDebug" else "assembleRelease");
    gradle.addArg(b.fmt("-PzigJniLibs={s}", .{b.pathResolve(&.{ root, jni_dir })}));
    gradle.step.dependOn(&jni_stage.step);
    // Gradle writes into android/app/build, which zig cannot track.
    gradle.has_side_effects = true;

    const kind: []const u8 = if (optimize == .Debug) "debug" else "release";
    const apk_name = b.fmt("app-{s}.apk", .{kind});
    const apk_install = b.addUpdateSourceFiles();
    apk_install.addCopyFileToSource(
        b.path(b.fmt("android/app/build/outputs/apk/{s}/{s}", .{ kind, apk_name })),
        b.fmt("{s}/apk/{s}", .{ install_prefix, apk_name }),
    );
    apk_install.step.dependOn(&gradle.step);
    b.getInstallStep().dependOn(&apk_install.step);
}

/// Gradle locates the Android SDK through android/local.properties, which is
/// machine-specific and gitignored. If it is missing or points at a directory
/// that does not exist on this machine (e.g. a devcontainer path), regenerate
/// it from the environment.
fn ensureAndroidSdkConfigured(b: *std.Build, ndk: []const u8) void {
    const io = b.graph.io;
    const cwd = std.Io.Dir.cwd();
    const root = rootPath(b);
    const props_path = b.pathJoin(&.{ root, "android", "local.properties" });

    if (cwd.readFileAlloc(io, props_path, b.allocator, .unlimited)) |contents| {
        if (sdkDirFromProperties(contents)) |sdk_dir| {
            if (cwd.access(io, sdk_dir, .{})) |_| return else |_| {}
        }
    } else |_| {}

    const env = &b.graph.environ_map;
    const derived: ?[]const u8 = blk: {
        // <sdk>/ndk/<version> is the SDK-managed NDK layout.
        const ndk_parent = std.fs.path.dirname(ndk) orelse break :blk null;
        if (!std.mem.eql(u8, std.fs.path.basename(ndk_parent), "ndk")) break :blk null;
        break :blk std.fs.path.dirname(ndk_parent);
    };
    const sdk = env.get("ANDROID_HOME") orelse env.get("ANDROID_SDK_ROOT") orelse derived orelse
        std.process.fatal("cannot determine the Android SDK location for gradle: set ANDROID_HOME " ++
            "or write android/local.properties with sdk.dir=", .{});
    cwd.access(io, sdk, .{}) catch
        std.process.fatal("Android SDK not found at '{s}'", .{sdk});

    // Forward slashes: backslashes are escape characters in .properties files.
    const sdk_fwd = b.allocator.dupe(u8, sdk) catch @panic("OOM");
    std.mem.replaceScalar(u8, sdk_fwd, '\\', '/');
    cwd.writeFile(io, .{
        .sub_path = props_path,
        .data = b.fmt("# This file is automatically generated. Do not commit to version control.\nsdk.dir={s}\n", .{sdk_fwd}),
    }) catch |err|
        std.process.fatal("unable to write android/local.properties: {s}", .{@errorName(err)});
}

fn sdkDirFromProperties(contents: []const u8) ?[]const u8 {
    var it = std.mem.splitScalar(u8, contents, '\n');
    while (it.next()) |raw_line| {
        const line = std.mem.trim(u8, raw_line, " \t\r");
        if (std.mem.startsWith(u8, line, "sdk.dir=")) return line["sdk.dir=".len..];
    }
    return null;
}

/// The NDK's libunwind.a lives in the clang resource directory, whose path
/// contains the clang major version; find it instead of hard-coding it.
fn clangRuntimeLibDir(b: *std.Build, prebuilt: []const u8, arch: std.Target.Cpu.Arch) ?[]const u8 {
    const io = b.graph.io;
    const clang_root = b.pathJoin(&.{ prebuilt, "lib", "clang" });
    var dir = std.Io.Dir.cwd().openDir(io, clang_root, .{ .iterate = true }) catch return null;
    defer dir.close(io);
    var it = dir.iterate();
    while (it.next(io) catch null) |entry| {
        if (entry.kind != .directory) continue;
        const candidate = b.pathJoin(&.{ clang_root, entry.name, "lib", "linux", @tagName(arch) });
        std.Io.Dir.cwd().access(io, candidate, .{}) catch continue;
        return candidate;
    }
    return null;
}

// -- Emscripten ----------------------------------------------------------------

fn buildEmscripten(
    b: *std.Build,
    mod: *std.Build.Module,
    optimize: std.builtin.OptimizeMode,
    lib_rel: []const u8,
    install_prefix: []const u8,
) void {
    const em_root = resolveEmscriptenRoot(b);

    // Zig has no libc headers for emscripten; use the emsdk sysroot's.
    mod.addSystemIncludePath(b.graph.cwdRelativePath(
        b.pathJoin(&.{ em_root, "cache", "sysroot", "include" }),
    ));

    // Zig cannot drive the emscripten linker, so compile the app to a static
    // library and let emcc do the final link (it supplies the GLFW/GL/libc/libc++
    // implementations and emits the .html/.js/.wasm triple).
    const lib = b.addLibrary(.{
        .linkage = .static,
        .name = app_name,
        .root_module = mod,
    });

    const emcc = b.addSystemCommand(&.{emccExecutable(b, em_root)});
    const opt_flags: []const []const u8 = switch (optimize) {
        .Debug => &.{ "-O0", "-g", "-sASSERTIONS=1" },
        .ReleaseSmall => &.{"-Oz"},
        else => &.{"-O3"},
    };
    emcc.addArgs(opt_flags);
    emcc.addArgs(&.{ "-sUSE_GLFW=3", "-sASYNCIFY" });
    emcc.addArtifactArg(lib);
    emcc.addFileArg(b.path(b.pathJoin(&.{ lib_rel, "libraylib.a" })));
    emcc.addArg("-o");
    const html = emcc.addOutputFileArg(app_name ++ ".html");

    // emcc emits the .js and .wasm next to the requested .html output.
    const install_files = b.addUpdateSourceFiles();
    installFile(b, install_files, html, install_prefix, "web", app_name ++ ".html");
    installFile(b, install_files, outputSibling(html, app_name ++ ".js"), install_prefix, "web", app_name ++ ".js");
    installFile(b, install_files, outputSibling(html, app_name ++ ".wasm"), install_prefix, "web", app_name ++ ".wasm");
    b.getInstallStep().dependOn(&install_files.step);
}

/// References a file emitted into the same generated directory as `output`
/// (an addOutputFileArg result) without being named on the command line.
fn outputSibling(output: std.Build.LazyPath, basename: []const u8) std.Build.LazyPath {
    return .{ .generated = .{
        .index = output.generated.index,
        .up = output.generated.up + 1,
        .sub_path = basename,
    } };
}

const EmscriptenRoot = struct { path: []const u8, source: []const u8 };

fn findEmscriptenRoot(b: *std.Build) ?EmscriptenRoot {
    const env = &b.graph.environ_map;
    if (env.get("EMSCRIPTEN_ROOT")) |root| return .{ .path = root, .source = "EMSCRIPTEN_ROOT" };
    if (env.get("EMSDK")) |emsdk| return .{
        .path = b.pathJoin(&.{ emsdk, "upstream", "emscripten" }),
        .source = "EMSDK",
    };
    if (b.findProgram(.{ .names = &.{"emcc"} })) |emcc| {
        if (std.fs.path.dirname(emcc)) |dir| return .{ .path = dir, .source = "PATH" };
    }
    return null;
}

/// Like findEmscriptenRoot, but verifies emcc is actually present so a cloned
/// but never installed/activated emsdk fails fast instead of deep inside vcpkg.
fn resolveEmscriptenRoot(b: *std.Build) []const u8 {
    const found = findEmscriptenRoot(b) orelse
        std.process.fatal("emscripten not found: set EMSDK or EMSCRIPTEN_ROOT, or put emcc on PATH", .{});
    const emcc = emccExecutable(b, found.path);
    std.Io.Dir.cwd().access(b.graph.io, emcc, .{}) catch
        std.process.fatal("emcc not found at '{s}' (from {s}); run 'emsdk install latest && " ++
            "emsdk activate latest', or fix the environment variable", .{ emcc, found.source });
    return found.path;
}

fn emccExecutable(b: *std.Build, em_root: []const u8) []const u8 {
    const io = b.graph.io;
    if (b.graph.host.result.os.tag == .windows) {
        for ([_][]const u8{ "emcc.bat", "emcc.exe" }) |name| {
            const path = b.pathJoin(&.{ em_root, name });
            if (std.Io.Dir.cwd().access(io, path, .{})) |_| return path else |_| {}
        }
    }
    return b.pathJoin(&.{ em_root, "emcc" });
}

// -- Target / triplet mapping ---------------------------------------------------

fn classify(t: std.Target) Platform {
    if (t.os.tag == .emscripten) return .emscripten;
    if (t.abi.isAndroid()) return .android;
    return switch (t.os.tag) {
        .windows => .windows,
        .linux => .linux,
        .macos => .macos,
        else => std.process.fatal("unsupported target os: {s}", .{@tagName(t.os.tag)}),
    };
}

fn vcpkgTriplet(platform: Platform, t: std.Target) []const u8 {
    return switch (platform) {
        .windows => switch (t.cpu.arch) {
            // The overlay triplet in cmake/triplets builds ports with zig
            // targeting x86_64-windows-gnu, matching zig's own windows ABI.
            .x86_64 => if (t.abi == .msvc)
                std.process.fatal("the msvc abi is not supported by zig build; use the default " ++
                    "gnu abi (or the CMake msvc presets)", .{})
            else
                "x64-mingw-static",
            else => unsupportedArch(t),
        },
        .linux => switch (t.cpu.arch) {
            .x86_64 => "x64-linux",
            .aarch64 => "arm64-linux",
            else => unsupportedArch(t),
        },
        .macos => switch (t.cpu.arch) {
            .aarch64 => "arm64-osx",
            .x86_64 => "x64-osx",
            else => unsupportedArch(t),
        },
        .android => switch (t.cpu.arch) {
            .aarch64 => "arm64-android",
            .x86_64 => "x64-android",
            else => unsupportedArch(t),
        },
        .emscripten => "wasm32-emscripten",
    };
}

fn unsupportedArch(t: std.Target) noreturn {
    std.process.fatal("unsupported cpu arch {s} for os {s}", .{
        @tagName(t.cpu.arch), @tagName(t.os.tag),
    });
}

fn vcpkgHostTriplet(b: *std.Build) []const u8 {
    const host = b.graph.host.result;
    const arm = host.cpu.arch == .aarch64;
    return switch (host.os.tag) {
        .windows => if (arm) "arm64-windows" else "x64-windows",
        .linux => if (arm) "arm64-linux" else "x64-linux",
        .macos => if (arm) "arm64-osx" else "x64-osx",
        else => std.process.fatal("unsupported host os: {s}", .{@tagName(host.os.tag)}),
    };
}

// -- vcpkg install (configure time) ----------------------------------------------

/// Installs the manifest dependencies for `triplet` into `install_root`,
/// bootstrapping vcpkg first if needed. A stamp file keyed on the manifest
/// contents makes this a cheap no-op on subsequent builds.
fn ensureVcpkgInstalled(
    b: *std.Build,
    platform: Platform,
    triplet: []const u8,
    host_triplet: []const u8,
    install_root: []const u8,
) void {
    const io = b.graph.io;
    const cwd = std.Io.Dir.cwd();
    const host_is_windows = b.graph.host.result.os.tag == .windows;

    // Re-run the configure phase when the manifests or the stamp change. The
    // stamp lives inside the install root so deleting that root (or the whole
    // build/<target> dir) also deletes the stamp, which retriggers configure
    // even on a warm cache.
    const stamp_rel = b.fmt("{s}/.zig-stamp", .{install_root});
    b.dependOnFileContents(b.path("vcpkg.json"));
    b.dependOnFileContents(b.path(stamp_rel));

    const root = rootPath(b);
    const manifest = cwd.readFileAlloc(io, b.pathJoin(&.{ root, "vcpkg.json" }), b.allocator, .unlimited) catch |err|
        std.process.fatal("unable to read vcpkg.json: {s}", .{@errorName(err)});
    const config = cwd.readFileAlloc(io, b.pathJoin(&.{ root, "vcpkg-configuration.json" }), b.allocator, .unlimited) catch |err| blk: {
        // The overlay triplet that builds windows ports with zig is declared
        // there; without it vcpkg would silently fall back to the community
        // x64-mingw-static triplet and produce ABI-mismatched libraries.
        if (std.mem.eql(u8, triplet, "x64-mingw-static"))
            std.process.fatal("unable to read vcpkg-configuration.json (declares the zig overlay triplet): {s}", .{@errorName(err)});
        b.graph.poisonCache();
        break :blk "";
    };
    if (config.len != 0) b.dependOnFileContents(b.path("vcpkg-configuration.json"));

    var hasher = std.crypto.hash.sha2.Sha256.init(.{});
    hasher.update(install_root);
    hasher.update(&.{0});
    hasher.update(triplet);
    hasher.update(&.{0});
    hasher.update(manifest);
    hasher.update(&.{0});
    hasher.update(config);
    // The overlay triplets and the toolchain files they chainload feed vcpkg's
    // ABI hashes; editing them must invalidate the stamp or the app would
    // silently keep linking libraries built with the old settings.
    hashToolchainInputs(b, &hasher);
    var digest: [32]u8 = undefined;
    hasher.final(&digest);
    const want = std.fmt.bytesToHex(digest, .lower);

    const stamp_path = b.pathJoin(&.{ root, stamp_rel });
    const installed_include = b.pathJoin(&.{ root, install_root, triplet, "include" });
    if (cwd.readFileAlloc(io, stamp_path, b.allocator, .unlimited)) |have| {
        if (std.mem.eql(u8, have, &want)) {
            // Guard against a stale stamp when the installed tree was removed.
            if (cwd.access(io, installed_include, .{})) |_| return else |_| {}
        }
    } else |_| {}

    // Fail early, before a long vcpkg run, if a required SDK is missing.
    switch (platform) {
        .android => if (b.graph.environ_map.get("ANDROID_NDK_HOME") == null)
            std.process.fatal("ANDROID_NDK_HOME must point at an Android NDK to install the {s} triplet", .{triplet}),
        .emscripten => _ = resolveEmscriptenRoot(b),
        else => {},
    }

    // We are about to mutate vcpkg_installed/; never cache this configure run.
    b.graph.poisonCache();

    const vcpkg_exe = b.pathJoin(&.{ root, "vcpkg", if (host_is_windows) "vcpkg.exe" else "vcpkg" });
    if (cwd.access(io, vcpkg_exe, .{})) |_| {} else |_| {
        const bootstrap = b.pathJoin(&.{
            root, "vcpkg", if (host_is_windows) "bootstrap-vcpkg.bat" else "bootstrap-vcpkg.sh",
        });
        if (cwd.access(io, bootstrap, .{})) |_| {} else |_|
            std.process.fatal("vcpkg submodule is missing or empty; run: git submodule update --init", .{});
        std.debug.print("bootstrapping vcpkg...\n", .{});
        // Spawn the .bat directly: zig serializes it through the mitigated
        // cmd.exe form, which survives paths containing cmd metacharacters.
        if (host_is_windows) {
            runChecked(b, &.{ bootstrap, "-disableMetrics" }, root);
        } else {
            runChecked(b, &.{ "sh", bootstrap, "-disableMetrics" }, root);
        }
        if (cwd.access(io, vcpkg_exe, .{})) |_| {} else |_|
            std.process.fatal("vcpkg bootstrap did not produce {s}", .{vcpkg_exe});
    }

    std.debug.print("installing vcpkg dependencies for {s} (host {s})...\n", .{ triplet, host_triplet });
    // cwd = manifest root: vcpkg picks up vcpkg.json and the overlay triplets
    // declared in vcpkg-configuration.json, and resolves the install root
    // relative to it.
    runChecked(b, &.{
        vcpkg_exe,
        "install",
        b.fmt("--triplet={s}", .{triplet}),
        b.fmt("--host-triplet={s}", .{host_triplet}),
        b.fmt("--x-install-root={s}", .{install_root}),
    }, root);

    cwd.createDirPath(io, b.pathJoin(&.{ root, install_root })) catch {};
    cwd.writeFile(io, .{ .sub_path = stamp_path, .data = &want }) catch |err|
        std.process.fatal("unable to write vcpkg stamp: {s}", .{@errorName(err)});
}

/// Feeds the overlay triplets, the toolchain files they chainload, and the
/// zig compiler wrappers into the stamp hash, and registers them as configure
/// dependencies so edits retrigger both the configure phase and vcpkg install.
fn hashToolchainInputs(b: *std.Build, hasher: *std.crypto.hash.sha2.Sha256) void {
    const io = b.graph.io;
    const cwd = std.Io.Dir.cwd();
    const root = rootPath(b);
    const dirs = [_][]const u8{ "cmake/triplets", "cmake/toolchain", "cmake/compiler" };
    for (dirs) |rel_dir| {
        var dir = cwd.openDir(io, b.pathJoin(&.{ root, rel_dir }), .{ .iterate = true }) catch continue;
        defer dir.close(io);
        b.dependOnDirectory(b.path(rel_dir));

        var names: std.ArrayList([]const u8) = .empty;
        var it = dir.iterate();
        while (it.next(io) catch null) |entry| {
            if (entry.kind != .file) continue;
            names.append(b.allocator, b.dupe(entry.name)) catch @panic("OOM");
        }
        std.mem.sort([]const u8, names.items, {}, struct {
            fn lessThan(_: void, lhs: []const u8, rhs: []const u8) bool {
                return std.mem.lessThan(u8, lhs, rhs);
            }
        }.lessThan);

        for (names.items) |name| {
            const rel = b.pathJoin(&.{ rel_dir, name });
            b.dependOnFileContents(b.path(rel));
            const contents = cwd.readFileAlloc(io, b.pathJoin(&.{ root, rel }), b.allocator, .unlimited) catch continue;
            hasher.update(name);
            hasher.update(&.{0});
            hasher.update(contents);
            hasher.update(&.{0});
        }
    }
}

fn rootPath(b: *std.Build) []const u8 {
    const s = b.root.toString(b.allocator) catch @panic("OOM");
    return if (s.len == 0) "." else s;
}

fn runChecked(b: *std.Build, argv: []const []const u8, cwd_path: []const u8) void {
    const io = b.graph.io;
    var child = std.process.spawn(io, .{
        .argv = argv,
        .cwd = .{ .path = cwd_path },
        // The configure runner's stdout carries its configuration protocol;
        // pump the child's output to stderr so it reaches the console instead.
        .stdout = .pipe,
    }) catch |err| std.process.fatal("unable to spawn {s}: {s}", .{ argv[0], @errorName(err) });
    if (child.stdout) |out| {
        var buf: [4096]u8 = undefined;
        while (true) {
            const n = out.readStreaming(io, &.{&buf}) catch break;
            if (n == 0) break;
            std.debug.print("{s}", .{buf[0..n]});
        }
    }
    const term = child.wait(io) catch |err|
        std.process.fatal("failed waiting for {s}: {s}", .{ argv[0], @errorName(err) });
    if (!term.success()) std.process.fatal("command failed: {s}", .{argv[0]});
}
