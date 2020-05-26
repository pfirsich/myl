#include "util.hpp"

#include <SFML/Window.hpp>

double getTime()
{
    static sf::Clock clock;
    return clock.getElapsedTime().asSeconds();
}