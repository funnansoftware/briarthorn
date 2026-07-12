add_library(briarthorn-game STATIC)

target_sources(briarthorn-game
    PRIVATE
        Briarthorn.cpp
)

target_include_directories(briarthorn-game
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

if(EMSCRIPTEN)
    set(raylib_USE_STATIC_LIBS ON)
endif()

find_package(raylib CONFIG REQUIRED)

# PUBLIC: Briarthorn.hpp exposes raylib types (Vector2) to consumers.
target_link_libraries(briarthorn-game
    PUBLIC
        raylib
)

if(NOT EMSCRIPTEN AND NOT ANDROID)
    find_package(glfw3 CONFIG REQUIRED)

    # Link glfw AFTER raylib so the static linker can resolve raylib's
    # references to glfw symbols (single-pass left-to-right resolution).
    target_link_libraries(briarthorn-game
        PRIVATE
            glfw
    )
endif()

if(BUILD_TESTING AND NOT ANDROID AND NOT EMSCRIPTEN)
    add_subdirectory(test)
endif()
