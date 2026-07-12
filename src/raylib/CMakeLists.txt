project(briarthorn-raylib)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    Renderer.cpp
    Window.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

if(EMSCRIPTEN)
    set(raylib_USE_STATIC_LIBS ON)
endif()

find_package(raylib CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        briarthorn-game
        raylib
)

if(NOT EMSCRIPTEN AND NOT ANDROID)
    find_package(glfw3 CONFIG REQUIRED)

    # Link glfw AFTER raylib so the static linker can resolve raylib's
    # references to glfw symbols (single-pass left-to-right resolution).
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            glfw
    )
endif()

# if(BUILD_TESTING AND NOT ANDROID AND NOT EMSCRIPTEN)
#     add_subdirectory(test)
# endif()
