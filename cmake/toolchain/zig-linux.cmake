# vcpkg chainload toolchain that builds ports with zig targeting
# x86_64-linux-gnu, matching the application toolchain.
#
# Note: ports that need target-platform system headers (e.g. X11 for glfw3)
# still require them to be installed on the build machine; zig replaces the
# compiler, not the platform SDK.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

get_filename_component(_zig_compiler_dir "${CMAKE_CURRENT_LIST_DIR}/../compiler" ABSOLUTE)

# The wrappers come in pairs; pick by the machine running the build.
if(CMAKE_HOST_WIN32)
    set(_zig_ext ".bat")
else()
    set(_zig_ext ".sh")
endif()

set(CMAKE_C_COMPILER   "${_zig_compiler_dir}/zigcc${_zig_ext}")
set(CMAKE_CXX_COMPILER "${_zig_compiler_dir}/zig++${_zig_ext}")
set(CMAKE_C_COMPILER_TARGET   x86_64-linux-gnu)
set(CMAKE_CXX_COMPILER_TARGET x86_64-linux-gnu)

set(CMAKE_AR      "${_zig_compiler_dir}/zigar${_zig_ext}"     CACHE FILEPATH "" FORCE)
set(CMAKE_RANLIB  "${_zig_compiler_dir}/zigranlib${_zig_ext}" CACHE FILEPATH "" FORCE)
