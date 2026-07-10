# briarthorn

A cross-platform C++23 [raylib](https://www.raylib.com/) project that builds
for **Windows, Linux, macOS, Android, and the web (WebAssembly)** from a single
`zig build`, with dependencies managed by [vcpkg](https://github.com/microsoft/vcpkg).
A parallel [CMake presets](#building-with-cmake-presets) flow covers
platform-native toolchains (MSVC, GCC, Clang, NDK, Emscripten).

## Prerequisites

Everything needs:

- [git](https://git-scm.com/)
- [zig](https://ziglang.org/download/) — a master (nightly) build; the pinned
  version is in [.zigversion](.zigversion)

Per platform:

| Target | Extra requirements |
| --- | --- |
| Windows | none — vcpkg bootstraps itself on first build |
| Linux | X11 development headers: `sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config` |
| macOS | Xcode Command Line Tools (`xcode-select --install`) |
| Android | an [Android NDK](https://developer.android.com/ndk) (`ANDROID_NDK_HOME` pointing at it); for APK packaging also an Android SDK (`ANDROID_HOME`) and a JDK |
| Web | none — the emsdk submodule bootstraps automatically on the first web build (one-time toolchain download, ~1.5 GB). Set `EMSDK`/`EMSCRIPTEN_ROOT` or put `emcc` on `PATH` to use your own emsdk instead |

The [devcontainer](.devcontainer) ships all of the above for Linux hosts
(GCC 15, Clang 22, zig, NDK, Android SDK + emulator), so Linux, Android, and
web builds work in it out of the box.

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
libraries, so desktop targets are not cross-compilable.

```sh
zig build             # Debug build to build/<target>-debug/installed/bin/
zig build run         # build + run (extra args: zig build run -- <args>)
```

| Host | Output directory (Debug) | vcpkg triplet | Notes |
| --- | --- | --- | --- |
| Windows | `build/x86_64-windows-gnu-debug` | `x64-mingw-static` (overlay) | ports are built with zig targeting `x86_64-windows-gnu`; the MSVC ABI is not supported here (use the CMake MSVC presets) |
| Linux | `build/x86_64-linux-gnu-debug` | `x64-linux` / `arm64-linux` | |
| macOS | `build/aarch64-macos-debug` | `arm64-osx` / `x64-osx` | |

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
  `build/<target>-<optimize>/installed/lib/libhello-triangle.so` (API level 35
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

The first web build bootstraps the [emsdk](emsdk/) submodule (install +
activate, pinned by the submodule commit; the downloaded toolchain lives
gitignored inside the submodule). Produces
`build/wasm32-emscripten-<optimize>/installed/web/hello-triangle.{html,js,wasm}`.
Browsers won't load wasm from `file://`, so serve the directory:

```sh
python -m http.server -d build/wasm32-emscripten-debug/installed/web
# then open http://localhost:8000/hello-triangle.html
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
  `cmake/triplets`, `cmake/toolchain`, or `cmake/compiler` automatically
  re-runs the vcpkg install on the next build.

## Building with CMake presets

The CMake flow uses the same vcpkg manifest with platform-native toolchains.
Configure and build with a preset name:

```sh
cmake --preset x64-linux-clang-debug
cmake --build --preset x64-linux-clang-debug
```

Available presets (each in `-debug` and `-release` variants):

| Host | Presets |
| --- | --- |
| Windows | `x64-windows-msvc-*` (Visual Studio), `x64-windows-zig-*` (zig as compiler) |
| Linux | `x64-linux-gcc-*`, `x64-linux-clang-*`, `x64-linux-zig-*` |
| macOS | `arm64-macos-clang-*` (Homebrew clang) |
| Linux → Android | `x64-linux-android-ndk-*` |
| Linux → Web | `x64-linux-emcc-*` (uses the emsdk submodule; bootstrap it once with `./emsdk/emsdk install latest && ./emsdk/emsdk activate latest`, or run any zig web build first) |

Build output goes to `build/<preset>/`; `cmake --install build/<preset>`
places binaries under `build/<preset>/installed/`. If `vcpkg/vcpkg` (or
`vcpkg.exe`) is missing, run `./vcpkg/bootstrap-vcpkg.sh -disableMetrics` (or
the `.bat`) once first.

### Android APK

The Android presets add gradle-driven APK targets:

```sh
cmake --build --preset x64-linux-android-ndk-debug --target apk-debug
```

APKs are copied to `build/<preset>/installed/android/`.

## License

Non-commercial use — see [LICENSE.md](LICENSE.md).
