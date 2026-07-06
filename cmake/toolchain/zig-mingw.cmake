# vcpkg chainload toolchain that builds ports with zig targeting the
# x86_64-windows-gnu (MinGW) ABI, matching the application toolchain.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

get_filename_component(_zig_compiler_dir "${CMAKE_CURRENT_LIST_DIR}/../compiler" ABSOLUTE)

set(CMAKE_C_COMPILER   "${_zig_compiler_dir}/zigcc.bat")
set(CMAKE_CXX_COMPILER "${_zig_compiler_dir}/zig++.bat")
set(CMAKE_C_COMPILER_TARGET   x86_64-windows-gnu)
set(CMAKE_CXX_COMPILER_TARGET x86_64-windows-gnu)

set(CMAKE_AR      "${_zig_compiler_dir}/zigar.bat"     CACHE FILEPATH "" FORCE)
set(CMAKE_RANLIB  "${_zig_compiler_dir}/zigranlib.bat" CACHE FILEPATH "" FORCE)
