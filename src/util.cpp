#include "util.hpp"

#include <SFML/Window.hpp>

namespace myl {

double getTime()
{
    static sf::Clock clock;
    return clock.getElapsedTime().asSeconds();
}

}