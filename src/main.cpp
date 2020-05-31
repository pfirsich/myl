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

struct Color {
    myl::Color value;
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

template <typename Derived, typename Shape>
class ShapeRenderSystem {
public:
    ShapeRenderSystem(const std::string& componentName)
        : componentId_(myl::getComponentId(componentName))
        , shapes_(componentId_)
    {
    }

    void update(float /*dt*/)
    {
        static const auto cTransform = myl::getComponentId("Transform");
        static const auto cColor = myl::getComponentId("Color");
        for (auto entity : myl::getEntities(cTransform + componentId_)) {
            auto& shape = shapes_.template get<true>(entity);
            const auto trafo = myl::getComponent<Transform>(entity, cTransform);
            shape.setPosition(trafo->position.x, trafo->position.y);
            shape.setRotation(trafo->angle / M_PI * 180.0f);
            static_cast<Derived*>(this)->updateShape(entity, shape);
            if (myl::hasComponent(entity, cColor)) {
                auto& color = myl::getComponent<Color>(entity, cColor)->value;
                shape.setFillColor(sf::Color(static_cast<uint32_t>(color)));
            } else {
                shape.setFillColor(sf::Color(255, 255, 255, 255));
            }
            window::getWindow().draw(shape);
        }
        shapes_.remove();
    }

private:
    myl::ComponentId componentId_;
    myl::SystemData<Shape> shapes_;
};

struct RectangleRenderSystem : public myl::RegisteredSystem<RectangleRenderSystem>,
                               public ShapeRenderSystem<RectangleRenderSystem, sf::RectangleShape> {
    inline static const std::string name = "RectangleRender";

    RectangleRenderSystem()
        : ShapeRenderSystem("RectangleRender")
    {
    }

    void updateShape(myl::EntityId entity, sf::RectangleShape& shape) const
    {
        static const auto cRectangleRender = myl::getComponentId("RectangleRender");
        const auto rectangleRender = myl::getComponent<RectangleRender>(entity, cRectangleRender);
        shape.setSize(sf::Vector2f(rectangleRender->size.x, rectangleRender->size.y));
    }
};

struct CircleRenderSystem : public myl::RegisteredSystem<CircleRenderSystem>,
                            public ShapeRenderSystem<CircleRenderSystem, sf::CircleShape> {
    inline static const std::string name = "CircleRender";

    CircleRenderSystem()
        : ShapeRenderSystem("CircleRender")
    {
    }

    void updateShape(myl::EntityId entity, sf::CircleShape& shape) const
    {
        static const auto cCircleRender = myl::getComponentId("CircleRender");
        const auto circleRender = myl::getComponent<CircleRender>(entity, cCircleRender);
        shape.setRadius(circleRender->radius);
    }
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
    myl::registerComponent("Color", myl::StructBuilder().addField("value", &Color::value).build());

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
