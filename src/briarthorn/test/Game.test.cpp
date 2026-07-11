#include <gtest/gtest.h>

#include <briarthorn/Game.hpp>

namespace
{

    TEST(GameTest, StoresSizeAndTitle)
    {
        const briarthorn::Game game{{.width = 320, .height = 240}, "briarthorn"};

        EXPECT_EQ(game.size().width, 320);
        EXPECT_EQ(game.size().height, 240);
        EXPECT_EQ(game.title(), "briarthorn");
    }

    TEST(GameTest, TriangleMatchesTheOriginalHelloTriangleLayout)
    {
        const briarthorn::Game game{{.width = 800, .height = 600}, "test"};

        const briarthorn::Triangle triangle = game.triangle();

        EXPECT_FLOAT_EQ(triangle.top.x, 400.0F);
        EXPECT_FLOAT_EQ(triangle.top.y, 100.0F);
        EXPECT_FLOAT_EQ(triangle.left.x, 300.0F);
        EXPECT_FLOAT_EQ(triangle.left.y, 300.0F);
        EXPECT_FLOAT_EQ(triangle.right.x, 500.0F);
        EXPECT_FLOAT_EQ(triangle.right.y, 300.0F);
    }

    TEST(GameTest, TriangleIsCenteredAndSymmetricForAnyWindowSize)
    {
        const briarthorn::Game game{{.width = 1024, .height = 768}, "test"};

        const briarthorn::Triangle triangle = game.triangle();
        const float centerX = 512.0F;

        EXPECT_FLOAT_EQ(triangle.top.x, centerX);
        EXPECT_FLOAT_EQ(centerX - triangle.left.x, triangle.right.x - centerX);
        EXPECT_FLOAT_EQ(triangle.left.y, triangle.right.y);
        EXPECT_LT(triangle.top.y, triangle.left.y);
    }

} // namespace
