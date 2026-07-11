# briarthorn

[![windows](https://github.com/funnansoftware/briarthorn/actions/workflows/windows.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/windows.yml)
[![macos](https://github.com/funnansoftware/briarthorn/actions/workflows/macos.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/macos.yml)
[![linux](https://github.com/funnansoftware/briarthorn/actions/workflows/linux.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/linux.yml)
[![coverage](https://github.com/funnansoftware/briarthorn/actions/workflows/coverage.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/coverage.yml)
[![web](https://github.com/funnansoftware/briarthorn/actions/workflows/web.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/web.yml)
[![android](https://github.com/funnansoftware/briarthorn/actions/workflows/android.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/android.yml)

A cross-platform C++23 [raylib](https://www.raylib.com/) application that runs on
**Windows, Linux, macOS, Android, and the web (WebAssembly)**, with dependencies
managed by [vcpkg](https://github.com/microsoft/vcpkg).

There are two ways to build it — you only need the tools for the one you use:

- **[`zig build`](#building-with-zig)** — the simplest path: one command builds
  any target, and vcpkg bootstraps itself on the first run.
- **[CMake presets](#building-with-cmake-presets)** — builds with your
  platform's native toolchain (Visual Studio, GCC, Clang, the Android NDK,
  Emscripten), or with zig used as the compiler.

## Prerequisites

### Everyone needs

- [git](https://git-scm.com/)
- The system libraries raylib and its windowing layer (GLFW) are compiled
  against — vcpkg builds them from source, so these must be present first:

  | OS | Install |
  | --- | --- |
  | Linux | `sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config` |
  | macOS | Xcode Command Line Tools: `xcode-select --install` |
  | Windows | nothing extra — the Windows SDK that ships with Visual Studio covers it |

### To build with zig

- [zig](https://ziglang.org/download/) — a master (nightly) build; the exact
  pinned version is in [.zigversion](.zigversion)

### To build with CMake presets

- [CMake](https://cmake.org/download/) and [Ninja](https://github.com/ninja-build/ninja/releases)
- a compiler for the preset you pick — everything targets 64-bit:

  | Preset family | Compiler to install |
  | --- | --- |
  | `linux-gcc-*` | GCC (`g++`), recent enough for C++23 — the devcontainer uses GCC 15 |
  | `linux-clang-*` | Clang (`clang++`), recent enough for C++23 — the devcontainer uses Clang 22 |
  | `windows-msvc-*` | [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) with the "Desktop development with C++" workload |
  | `macos-clang-*` | Homebrew LLVM/Clang, pinned to version 21: `brew install llvm@21` |
  | `*-zig-*` | [zig](https://ziglang.org/download/) — used as the C/C++ compiler (targets MinGW on Windows) |

### For web or Android targets (either build path)

| Target | Extra requirements |
| --- | --- |
| Web | none — the first web build downloads and installs emscripten from the [emsdk](emsdk/) submodule (one-time, ~1.5 GB) into the git-ignored, per-host prefix `.emsdk/<host>`. Set `EMSDK`/`EMSCRIPTEN_ROOT`, or put `emcc` on `PATH`, to use your own install instead |
| Android | an [Android NDK](https://developer.android.com/ndk) (point `ANDROID_NDK_HOME` at it); for APK packaging also an Android SDK (`ANDROID_HOME`) and a JDK |

The [devcontainer](.devcontainer) ships all of the above for Linux hosts (git,
CMake, Ninja, GCC 15, Clang 22, zig, the Android NDK, Android SDK + emulator,
and a JDK), so Linux, Android, and web builds work in it out of the box.

## Cloning

vcpkg is a submodule, so clone with:

```sh
git clone --recurse-submodules https://github.com/funnansoftware/briarthorn.git
```

Already cloned without submodules? Run `git submodule update --init`.

## Building with zig

There is no separate configure step: `zig build` selects the vcpkg triplet for
the target, installs the manifest dependencies (bootstrapping vcpkg first if
needed), and compiles the app. Like the CMake presets, each target+optimize
combination gets its own directory under `build/`, named after the resolved
target triple and the optimize mode (e.g. `build/x86_64-windows-gnu-debug`,
`build/aarch64-linux-android-releasesmall`):

```
build/<target>-<optimize>/vcpkg_installed/   vcpkg packages for the target's triplet
build/<target>-<optimize>/installed/         final binaries (bin/, lib/, apk/, web/)
```

The first build of a target compiles its dependencies (or restores them from
the local vcpkg binary cache); after that, builds are incremental and vcpkg is
not re-run until `vcpkg.json` or the toolchain files change.

### Desktop (native)

Run these on the platform you are targeting — vcpkg builds host-native
libraries, so desktop targets are not cross-compilable (`zig build` refuses
early with an explanation; use the devcontainer or WSL for Linux builds from
Windows). Android and web targets cross-compile from any host.

```sh
zig build             # Debug build to build/<target>-debug/installed/bin/
zig build run         # build + run (extra args: zig build run -- <args>)
```

| Host | Output directory (Debug) | vcpkg triplet | Notes |
| --- | --- | --- | --- |
| Windows | `build/x86_64-windows-gnu-debug` | `x64-mingw-static` (overlay) | ports are built with zig targeting `x86_64-windows-gnu`; the MSVC ABI is not supported here (use the CMake MSVC presets) |
| Linux | `build/x86_64-linux-gnu-debug` | `x64-linux-zig` / `arm64-linux-zig` (overlay) | ports are built with zig via the overlay triplet |
| macOS | `build/aarch64-macos-debug` | `arm64-osx-zig` / `x64-osx-zig` (overlay) | ports are built with zig via the overlay triplet (zig cc locates the macOS SDK); build natively per architecture |

### Android

```sh
zig build -Dtarget=aarch64-linux-android                 # device (arm64-v8a)
zig build -Dtarget=x86_64-linux-android                  # emulator
zig build -Dtarget=aarch64-linux-android -Dandroid-api=30
```

Requires `ANDROID_NDK_HOME`, plus an Android SDK and a JDK for APK packaging
(the SDK is located via `ANDROID_HOME` or `android/local.properties`, which is
regenerated automatically when stale). Produces:

- the NativeActivity library
  `build/<target>-<optimize>/installed/lib/libbriarthorn-app.so` (API level 35
  by default, matching the CMake presets), and
- an APK at `build/<target>-<optimize>/installed/apk/app-debug.apk`
  (`app-release.apk` for release optimize modes), packaged by the gradle
  project in [android/](android/) from the zig-built library — gradle's own
  CMake native build is skipped.

Install it on a device or emulator with
`adb install build/aarch64-linux-android-debug/installed/apk/app-debug.apk`.

### Web

```sh
zig build -Dtarget=wasm32-emscripten
```

The first web build bootstraps emscripten from the [emsdk](emsdk/) submodule
(install + activate, pinned by the submodule commit; the downloaded toolchain
lives gitignored in `.emsdk/<host>`, keyed by host OS so a tree shared across
operating systems keeps one install per platform). Produces
`build/wasm32-emscripten-<optimize>/installed/web/briarthorn-app.{html,js,wasm}`.
Browsers won't load wasm from `file://`, so serve the directory:

```sh
python -m http.server -d build/wasm32-emscripten-debug/installed/web
# then open http://localhost:8000/briarthorn-app.html
```

### Options

| Option | Meaning |
| --- | --- |
| `-Doptimize=Debug\|ReleaseSafe\|ReleaseFast\|ReleaseSmall` | optimization mode (default `Debug`; Debug links vcpkg's debug libraries). Each mode installs to its own `build/<target>-<optimize>/` tree |
| `-Dandroid-api=<n>` | Android API level (default 35) |
| `-Dvcpkg-triplet=<t>` | override the vcpkg triplet (you own ABI compatibility) |

Notes:

- Each build directory has its own `vcpkg_installed/`, so switching targets or
  optimize modes never invalidates another configuration. Deleting a
  `build/<target>-<optimize>/` directory is safe — the next build reinstalls
  everything (usually restored from vcpkg's binary cache in seconds).
- Editing `vcpkg.json`, `vcpkg-configuration.json`, or anything under
  `cmake/triplets`, `cmake/toolchain`, or `scripts` (the zig compiler wrappers)
  automatically re-runs the vcpkg install on the next build.

## Building with CMake presets

The CMake flow uses the same vcpkg manifest with platform-native toolchains.
Configure and build with a preset name:

```sh
cmake --preset linux-clang-debug
cmake --build --preset linux-clang-debug
```

Available presets (each in `-debug` and `-release` variants):

| Host | Presets |
| --- | --- |
| Windows | `windows-msvc-*` (Visual Studio), `windows-zig-*` (zig as compiler) |
| Linux | `linux-gcc-*`, `linux-clang-*`, `linux-zig-*` |
| macOS | `macos-clang-*` (Homebrew LLVM) |
| Linux → Android | `android-*` |
| Linux → Web | `web-*` (uses the per-host emscripten install in `.emsdk/`; bootstrap it once with `./scripts/bootstrap-emsdk.sh`, or run any zig web build first) |

`linux-clang` and `macos-clang` also have a `-coverage` variant that builds an
instrumented binary and reports coverage (`--target llvm-coverage`).

Build output goes to `build/<preset>/`; `cmake --install build/<preset>`
places binaries under `build/<preset>/installed/`. If `vcpkg/vcpkg` (or
`vcpkg.exe`) is missing, run `./vcpkg/bootstrap-vcpkg.sh -disableMetrics` (or
the `.bat`) once first.

### Android APK

The Android presets add a gradle-driven `apk` target (the debug/release variant
follows the preset's build type):

```sh
cmake --build --preset android-debug --target apk
```

The APK is copied into the build tree at
`build/<preset>/outputs/apk/<variant>/app-<variant>.apk`.

## License

Non-commercial use — see [LICENSE.md](LICENSE.md).
