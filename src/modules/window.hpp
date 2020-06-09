#pragma once

#include <string>

#include <SFML/Graphics.hpp>

namespace myl {
namespace modules {
    namespace window {
        // This is not part of the service, but some hacky shit so I can render stuff
        sf::RenderWindow& getWindow();

        void init(const std::string& title, size_t width, size_t height, bool fullscreen);
        void setTitle(const std::string& title);
        void setVSync(bool enabled);
        bool update();
        void clear();
        void present();
        sf::Font& getDefaultFont();
    }
}
}
