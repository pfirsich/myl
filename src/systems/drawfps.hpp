#pragma once

#include <SFML/Graphics.hpp>

#include "ecs.hpp"

namespace myl {
class DrawFpsSystem : public myl::RegisteredSystem<DrawFpsSystem> {
public:
    inline static const std::string name = "DrawFps";

    DrawFpsSystem();

    void update(float dt);

private:
    sf::Text text_;
    size_t frameCounter_;
    double nextCalcFps_;
};
}
