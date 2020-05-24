#include "window.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace myl {
namespace modules {
    namespace window {
        namespace {
            template <typename... Args>
            std::unique_ptr<sf::RenderWindow>& getWindowPtr()
            {
                static std::unique_ptr<sf::RenderWindow> window(nullptr);
                return window;
            }
        }

        void init(const std::string& name, size_t width, size_t height, bool fullscreen)
        {
            getWindowPtr() = std::make_unique<sf::RenderWindow>(
                sf::VideoMode(width, height), name, sf::Style::Default);
        }

        void setTitle(const std::string& title)
        {
            getWindowPtr()->setTitle(title);
        }

        bool update()
        {
            auto& window = *getWindowPtr();
            const auto open = window.isOpen();
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }
            return open;
        }

        void clear()
        {
            getWindowPtr()->clear(sf::Color::Black);
        }

        void present()
        {
            getWindowPtr()->display();
        }
    }
}
}