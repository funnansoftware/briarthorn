#include <print>

#include <raylib.h>

auto main() -> int
{
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 600;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, "Hello, Triangle");

    constexpr Vector2 topVertex{.x = 400, .y = 100};
    constexpr Vector2 leftVertex{.x = 300, .y = 300};
    constexpr Vector2 rightVertex{.x = 500, .y = 300};

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GRAY);
        DrawTriangle(topVertex, leftVertex, rightVertex, ORANGE);
        EndDrawing();
        PollInputEvents();
        SwapScreenBuffer();
    }

    CloseWindow();

    return EXIT_SUCCESS;
}