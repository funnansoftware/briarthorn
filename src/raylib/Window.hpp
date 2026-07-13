#pragma once

#include <raylib.h>
#include <string>

namespace bt::raylib
{
    class Window
    {
    public:
        struct Traits
        {
            std::string title;
            int width;
            int height;
        };

        Window(const Traits& traits);
        ~Window();

        auto shouldClose() const -> bool;

        auto begin(Color clear) const -> void;
        auto end() const -> void;
        auto size() const -> Vector2;

    private:
        std::string title_;
        int width_;
        int height_;
    };
}