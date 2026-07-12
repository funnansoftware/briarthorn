#pragma once

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

    private:
        std::string title_;
        int width_;
        int height_;
    };
}