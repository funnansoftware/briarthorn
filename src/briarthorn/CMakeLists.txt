add_library(briarthorn STATIC)

target_sources(briarthorn
    PRIVATE
        Briarthorn.cpp
)

target_include_directories(briarthorn
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

if(EMSCRIPTEN)
    set(raylib_USE_STATIC_LIBS ON)
endif()

find_package(raylib CONFIG REQUIRED)

# PUBLIC: Game.hpp exposes raylib types (Vector2) to consumers.
target_link_libraries(briarthorn
    PUBLIC
        raylib
)

if(NOT EMSCRIPTEN AND NOT ANDROID)
    find_package(glfw3 CONFIG REQUIRED)

    # Link glfw AFTER raylib so the static linker can resolve raylib's
    # references to glfw symbols (single-pass left-to-right resolution).
    target_link_libraries(briarthorn
        PRIVATE
            glfw
    )
endif()

if(BUILD_TESTING AND NOT ANDROID AND NOT EMSCRIPTEN)
    add_subdirectory(test)
endif()
