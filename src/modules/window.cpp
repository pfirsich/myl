#include "window.hpp"

#include <cassert>

#include <SFML/Window.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include "defaultfont.hpp"
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
            const auto dpr = getDevicePixelRatio();

            if (fullscreen) {
                const auto mode = sf::VideoMode::getDesktopMode();
                width = mode.width;
                height = mode.height;
            } else {
                width = width * dpr;
                height = height * dpr;
            }

            getWindowPtr() = std::make_unique<sf::RenderWindow>(sf::VideoMode(width, height), name,
                fullscreen ? sf::Style::Fullscreen : sf::Style::Default);
            auto& window = getWindow();
            window.setView(sf::View(sf::FloatRect(0, 0, width / dpr, height / dpr)));
            ImGui::SFML::Init(window);

            setImguiStyle();

            ImGui::GetStyle().ScaleAllSizes(dpr);
            ImGui::GetIO().FontGlobalScale = dpr;

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
                    const auto dpr = getDevicePixelRatio();
                    sf::FloatRect visibleArea(
                        0, 0, event.size.width / dpr, event.size.height / dpr);
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

        sf::Font& getDefaultFont()
        {
            static sf::Font font;
            static bool loaded = false;
            if (!loaded) {
                assert(font.loadFromMemory(myl::defaultFont, myl::defaultFontLen));
                loaded = true;
            }
            return font;
        }

        // How many device pixels per logical pixel?
        float getDevicePixelRatio()
        {
#if __APPLE__
            const auto mode = sf::VideoMode::getDesktopMode();
            if (mode.width > 1920) {
                // Apple and more then full HD? Must be retina aka hight DPI (#science)
                return 2.f;
            }
#endif
            return 1.f;
        }

    }
}
}
