#include "window.hpp"

#include <SFML/Window.hpp>

#include "input.hpp"

namespace myl {
namespace modules {
    namespace window {
        namespace {
            std::unique_ptr<sf::RenderWindow>& getWindowPtr()
            {
                static std::unique_ptr<sf::RenderWindow> window(nullptr);
                return window;
            }
        }

        sf::RenderWindow& getWindow()
        {
            return *getWindowPtr();
        }

        void init(const std::string& name, size_t width, size_t height, bool fullscreen)
        {
            getWindowPtr() = std::make_unique<sf::RenderWindow>(
                sf::VideoMode(width, height), name, sf::Style::Default);
        }

        void setTitle(const std::string& title)
        {
            getWindow().setTitle(title);
        }

        bool update()
        {
            auto& window = getWindow();
            const auto open = window.isOpen();
            input::internal::saveLastState();
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
                if (event.type == sf::Event::KeyPressed)
                    input::internal::setState(static_cast<input::Key>(event.key.code), true);
                if (event.type == sf::Event::KeyReleased)
                    input::internal::setState(static_cast<input::Key>(event.key.code), false);
            }
            return open;
        }

        void clear()
        {
            getWindow().clear(sf::Color::Black);
        }

        void present()
        {
            getWindow().display();
        }
    }
}
}