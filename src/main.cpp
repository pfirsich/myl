#include <cmath>

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <imgui.h>

#include "debugsystem.hpp"
#include "lua/lua.hpp"
#include "modules/input.hpp"
#include "modules/timer.hpp"
#include "modules/window.hpp"
#include "myl.hpp"

using namespace myl::modules;

struct Transform {
    glm::vec2 position;
    float angle;
};

struct Velocity {
    glm::vec2 value;
};

struct PlayerInputState {
    glm::vec2 moveDir;
};

struct RectangleRender {
    glm::vec2 size;
};

struct CircleRender {
    float radius;
};

float floatKey(input::Key key)
{
    return input::getKeyboardDown(key) ? 1.0f : 0.0f;
}

static sf::Font& getFont()
{
    static sf::Font font;
    assert(font.loadFromFile("RobotoMono-Regular.ttf"));
    return font;
}

struct PlayerInputSystem : public myl::RegisteredSystem<PlayerInputSystem> {
    inline static const std::string name = "PlayerInput";

    void update(float /*dt*/)
    {
        const auto cPlayerInputState = myl::getComponentId("PlayerInputState");
        for (auto entity : myl::getEntities(cPlayerInputState)) {
            auto input = myl::getComponent<PlayerInputState>(entity, cPlayerInputState);
            const auto lr = floatKey(input::Key::right) - floatKey(input::Key::left);
            const auto ud = floatKey(input::Key::down) - floatKey(input::Key::up);
            const auto moveDir = glm::vec2(lr, ud);
            input->moveDir = moveDir / (glm::length(moveDir) + 1.0e-9f);
        }
    }
};

class RectangleRenderSystem : public myl::RegisteredSystem<RectangleRenderSystem> {
public:
    inline static const std::string name = "RectangleRender";

    RectangleRenderSystem()
        : shapes_(myl::getComponentId("RectangleRender"))
    {
    }

    void update(float /*dt*/)
    {
        const auto cTransform = myl::getComponentId("Transform");
        const auto cRectangleRender = myl::getComponentId("RectangleRender");
        for (auto entity : myl::getEntities(cTransform + cRectangleRender)) {
            auto& shape = shapes_.get<true>(entity);
            auto trafo = myl::getComponent<Transform>(entity, cTransform);
            auto rectangleRender = myl::getComponent<RectangleRender>(entity, cRectangleRender);
            shape.setPosition(trafo->position.x, trafo->position.y);
            shape.setRotation(trafo->angle / M_PI * 180.0f);
            shape.setSize(sf::Vector2f(rectangleRender->size.x, rectangleRender->size.y));
            window::getWindow().draw(shape);
        }
        shapes_.remove();
    }

private:
    myl::SystemData<sf::RectangleShape> shapes_;
};

class CircleRenderSystem : public myl::RegisteredSystem<CircleRenderSystem> {
public:
    inline static const std::string name = "CircleRender";

    CircleRenderSystem()
        : shapes_(myl::getComponentId("CircleRender"))
    {
    }

    void update(float /*dt*/)
    {
        const auto cTransform = myl::getComponentId("Transform");
        const auto cCircleRender = myl::getComponentId("CircleRender");
        for (auto entity : myl::getEntities(cTransform + cCircleRender)) {
            auto& shape = shapes_.get<true>(entity);
            auto trafo = myl::getComponent<Transform>(entity, cTransform);
            auto circleRender = myl::getComponent<CircleRender>(entity, cCircleRender);
            shape.setPosition(trafo->position.x, trafo->position.y);
            shape.setRotation(trafo->angle / M_PI * 180.0f);
            shape.setRadius(circleRender->radius);
            window::getWindow().draw(shape);
        }
        shapes_.remove();
    }

private:
    myl::SystemData<sf::CircleShape> shapes_;
};

class DrawFpsSystem : public myl::RegisteredSystem<DrawFpsSystem> {
public:
    inline static const std::string name = "DrawFps";

    DrawFpsSystem()
        : frameCounter_(0)
        , nextCalcFps_(timer::getTime() + 1.0)
    {
        text_.setFont(getFont());
        text_.setCharacterSize(14);
        text_.setFillColor(sf::Color::White);
    }

    void update(float /*dt*/)
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

private:
    sf::Text text_;
    size_t frameCounter_;
    double nextCalcFps_;
};

int main(int argc, char** argv)
{
    const auto args = std::vector<std::string> { argv + 1, argv + argc };

    if (!myl::initGameDirectory(args))
        return EXIT_FAILURE;

    myl::registerComponent("Transform",
        myl::StructBuilder()
            .addField("position", &Transform::position)
            .addField("angle", &Transform::angle)
            .build());

    myl::registerComponent(
        "RectangleRender", myl::StructBuilder().addField("size", &RectangleRender::size).build());

    myl::registerComponent(
        "CircleRender", myl::StructBuilder().addField("radius", &CircleRender::radius).build());

    PlayerInputSystem playerInput;
    RectangleRenderSystem rectangleRender;
    CircleRenderSystem circleRender;
    DrawFpsSystem drawFpsSystem;
    DebugSystem debugSystem;

    myl::lua::State lua;
    lua.init();
    if (lua.hasMain()) {
        if (!lua.executeMain())
            return EXIT_FAILURE;
    } else {
        std::cerr << "No main entry point found." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
