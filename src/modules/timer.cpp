#include "timer.hpp"

#include <SFML/Window.hpp>

namespace myl {
namespace modules {
    namespace timer {
        double getTime()
        {
            static sf::Clock clock;
            return clock.getElapsedTime().asSeconds();
        }

        double getDelta()
        {
            static sf::Clock clock;
            return clock.restart().asSeconds();
        }
    }
}
}