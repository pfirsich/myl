#include "util.hpp"

#include <SFML/Window.hpp>

namespace myl {

double getTime()
{
    static sf::Clock clock;
    return clock.getElapsedTime().asSeconds();
}

std::string hexString(const void* data, size_t size)
{
    static const char hexDigits[] = "0123456789ABCDEF";

    std::string out;
    out.reserve(size * 3);
    for (size_t i = 0; i < size; ++i) {
        const auto c = reinterpret_cast<const uint8_t*>(data)[i];
        out.push_back(hexDigits[c >> 4]);
        out.push_back(hexDigits[c & 15]);
    }
    return out;
}

}