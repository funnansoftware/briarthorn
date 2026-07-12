add_subdirectory(game)

if(ANDROID)
    # Android's NativeActivity loads the app as a shared library.
    add_library(briarthorn SHARED)
else()
    add_executable(briarthorn)
endif()

target_sources(briarthorn
    PRIVATE
        main.cpp
)

target_link_libraries(briarthorn
    PRIVATE
        briarthorn-game
)

if(EMSCRIPTEN)
    target_link_options(briarthorn
        PRIVATE
            -sUSE_GLFW=3
            -sASYNCIFY
    )

    set_target_properties(briarthorn PROPERTIES SUFFIX ".html")

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/briarthorn.html
        ${CMAKE_CURRENT_BINARY_DIR}/briarthorn.js
        ${CMAKE_CURRENT_BINARY_DIR}/briarthorn.wasm
        DESTINATION web
    )
elseif(ANDROID)
    find_library(log-lib log)
    find_library(android-lib android)
    find_library(GLESv2-lib GLESv2)
    find_library(EGL-lib EGL)
    find_library(OpenSLES-lib OpenSLES)

    target_link_libraries(briarthorn
        PRIVATE
            ${log-lib}
            ${android-lib}
            ${GLESv2-lib}
            ${EGL-lib}
            ${OpenSLES-lib}
            atomic
    )

    # Force-export ANativeActivity_onCreate from raylib's static lib so that
    # Android's NativeActivity can find the entry point in the shared library.
    target_link_options(briarthorn
        PRIVATE
            -u ANativeActivity_onCreate
    )
endif()

if(NOT EMSCRIPTEN)
    install(TARGETS briarthorn
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()
