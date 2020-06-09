#include "drawfps.hpp"

#include "modules/timer.hpp"
#include "modules/window.hpp"

using namespace myl::modules;

namespace myl {
DrawFpsSystem::DrawFpsSystem()
    : frameCounter_(0)
    , nextCalcFps_(timer::getTime() + 1.0)
{
    text_.setFont(window::getDefaultFont());
    text_.setCharacterSize(14);
    text_.setFillColor(sf::Color::White);
}

void DrawFpsSystem::update(float /*dt*/)
{
    const auto now = timer::getTime();
    frameCounter_++;
    if (nextCalcFps_ < now) {
        nextCalcFps_ = now + 1.0;
        text_.setString("FPS: " + std::to_string(frameCounter_));
        frameCounter_ = 0;
    }
    window::getWindow().draw(text_);
}
}
