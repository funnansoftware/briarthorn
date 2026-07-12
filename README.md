# briarthorn

[![windows](https://github.com/funnansoftware/briarthorn/actions/workflows/windows.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/windows.yml)
[![macos](https://github.com/funnansoftware/briarthorn/actions/workflows/macos.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/macos.yml)
[![linux](https://github.com/funnansoftware/briarthorn/actions/workflows/linux.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/linux.yml)
[![coverage](https://github.com/funnansoftware/briarthorn/actions/workflows/coverage.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/coverage.yml)
[![web](https://github.com/funnansoftware/briarthorn/actions/workflows/web.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/web.yml)
[![android](https://github.com/funnansoftware/briarthorn/actions/workflows/android.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/android.yml)
[![zig](https://github.com/funnansoftware/briarthorn/actions/workflows/zig.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/zig.yml)
[![steamos](https://github.com/funnansoftware/briarthorn/actions/workflows/steamos.yml/badge.svg?branch=main)](https://github.com/funnansoftware/briarthorn/actions/workflows/steamos.yml)

A cross-platform C++23 [raylib](https://www.raylib.com/) application for
**Windows, Linux, macOS, Android, and the web (WebAssembly)**, with dependencies
managed by [vcpkg](https://github.com/microsoft/vcpkg). Build it with a single
`zig build`, or with CMake presets.

## Prerequisites

Everyone needs [git](https://git-scm.com/) and the system libraries raylib's
windowing layer (GLFW) is built against:

| OS | Install |
| --- | --- |
| Linux | `sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config` |
| macOS | Xcode Command Line Tools: `xcode-select --install` |
| Windows | nothing extra — the Windows SDK from Visual Studio covers it |

Then install the tools for your build path:

- **zig** — a master (nightly) build of [zig](https://ziglang.org/download/); the
  pinned version is in [.zigversion](.zigversion).
- **CMake presets** — [CMake](https://cmake.org/download/),
  [Ninja](https://github.com/ninja-build/ninja/releases), and your platform's
  compiler: Visual Studio 2022 (Windows), GCC (Linux), or Homebrew LLVM
  (`brew install llvm@21`, macOS).

Web builds download emscripten automatically (one-time, ~1.5 GB); Android needs
an [NDK](https://developer.android.com/ndk) (plus an SDK and JDK to package an
APK). The [devcontainer](.devcontainer) ships everything for Linux hosts.

## Clone

vcpkg is a submodule, so clone recursively:

```sh
git clone --recurse-submodules https://github.com/funnansoftware/briarthorn.git
```

Already cloned without submodules? Run `git submodule update --init`.

## Build and run

### With zig

One command builds an optimized release and runs it:

```sh
zig build run
```

vcpkg bootstraps itself on the first build. Add `-Doptimize=Debug` for a debug build.
Run the GoogleTest suite (desktop hosts) with `zig build test`.

### With CMake presets

Use the shorthand preset for your OS — `windows`, `linux`, or `macos`:

```sh
cmake --preset linux                            # configure (release)
cmake --build --preset linux                    # build
cmake --build --preset linux --target install   # install
./build/linux/installed/bin/briarthorn-app      # run
```

`cmake --list-presets` lists every preset for your OS. The named ones select a
specific compiler or build type — e.g. `linux-clang-debug`, `windows-zig-release`,
`macos-clang-coverage`.

### Web and Android

zig cross-compiles these from any host:

```sh
# Web — build, then serve it and open in a browser.
zig build -Dtarget=wasm32-emscripten
python3 -m http.server -d build/wasm32-emscripten-releasefast/installed/web
# open http://localhost:8000/briarthorn-app.html

# Android — build the APK, then install it on a device or emulator.
zig build -Dtarget=aarch64-linux-android
adb install build/aarch64-linux-android-releasefast/installed/apk/app-release.apk
```

### Steam Deck (SteamOS)

SteamOS is x86_64 Linux, so the same `zig build` works — the catch is the **glibc
floor**. briarthorn's Linux toolchain links natively, so a binary's minimum glibc
equals the glibc of the machine it was built on, and the main devcontainer's
bleeding-edge glibc fails on the Deck with `GLIBC_x.xx not found`. The
[`steamos` devcontainer](.devcontainer/steamos) fixes this by building against the
Steam Linux Runtime 3.0 "sniper" SDK (Debian 11, glibc 2.31) — below the Deck's
floor, so the binary runs there directly and inside the Steam runtime container.

Open it in VS Code (**Dev Containers: Reopen in Container → briarthorn-steamos**),
then:

```sh
zig build        # release -> build/x86_64-linux-gnu-releasefast/installed
zig build test   # GoogleTest suite, run against the sniper glibc floor

# Confirm nothing newer than the sniper floor leaked in before shipping:
objdump -T build/x86_64-linux-gnu-releasefast/installed/bin/briarthorn-app \
  | grep -oE 'GLIBC_[0-9.]+' | sort -uV | tail -1   # expect GLIBC_2.31 or lower
```

Copy `build/x86_64-linux-gnu-releasefast/installed/` to the Deck and run
`bin/briarthorn-app` from Desktop Mode, or add it to Steam as a non-Steam game. CI
([steamos.yml](.github/workflows/steamos.yml)) builds this on every push and
uploads the install tree as an artifact.

## License

Non-commercial use — see [LICENSE.md](LICENSE.md).
