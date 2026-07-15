cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

project(briarthorn)

include(CTest)

option(ENABLE_COVERAGE "Compile with llvm source-based coverage instrumentation" OFF)

if(ENABLE_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "ENABLE_COVERAGE requires a Clang compiler (using ${CMAKE_CXX_COMPILER_ID}).")
    endif()

    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate)
endif()

include(cmake/qt-vcpkg-fixup.cmake)
include(cmake/target/apk.cmake)
include(cmake/target/clang-format.cmake)
include(cmake/target/clang-tidy.cmake)

add_subdirectory(src)

# After add_subdirectory(src) so targets have registered themselves in the
# PROJECT_COVERAGE_TARGETS global property this script reads.
include(cmake/target/llvm-coverage.cmake)
