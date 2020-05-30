#include "window.hpp"

#include <SFML/Window.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include "input.hpp"
#include "timer.hpp"

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

        void setImguiStyle(); // imguistyle.cpp

        sf::RenderWindow& getWindow()
        {
            return *getWindowPtr();
        }

        void init(const std::string& name, size_t width, size_t height, bool fullscreen)
        {
            getWindowPtr() = std::make_unique<sf::RenderWindow>(sf::VideoMode(width, height), name,
                fullscreen ? sf::Style::Fullscreen : sf::Style::Default);
            ImGui::SFML::Init(getWindow());
            setImguiStyle();
            ImGui::SFML::UpdateFontTexture();
        }

        void setTitle(const std::string& title)
        {
            getWindow().setTitle(title);
        }

        void setVSync(bool enabled)
        {
            getWindow().setVerticalSyncEnabled(enabled);
        }

        bool update()
        {
            static double lastTime = timer::getTime();
            const auto now = timer::getTime();
            const auto dt = now - lastTime;
            lastTime = now;

            auto& window = getWindow();
            const auto open = window.isOpen();
            input::internal::saveLastState();
            sf::Event event;
            while (window.pollEvent(event)) {
                ImGui::SFML::ProcessEvent(event);
                if (event.type == sf::Event::Closed) {
                    window.close();
                } else if (event.type == sf::Event::KeyPressed) {
                    input::internal::setState(static_cast<input::Key>(event.key.code), true);
                } else if (event.type == sf::Event::KeyReleased) {
                    input::internal::setState(static_cast<input::Key>(event.key.code), false);
                } else if (event.type == sf::Event::Resized) {
                    sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                    window.setView(sf::View(visibleArea));
                }
            }
            ImGui::SFML::Update(window, sf::seconds(dt));
            return open;
        }

        void clear()
        {
            getWindow().clear(sf::Color::Black);
        }

        void present()
        {
            ImGui::EndFrame();
            ImGui::SFML::Render(getWindow());
            getWindow().display();
        }
    }
}
}
