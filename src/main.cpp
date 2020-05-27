#include <cmath>

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <imgui.h>

#include "debugsystem.hpp"
#include "ecs.hpp"
#include "lua/lua.hpp"
#include "modules/input.hpp"
#include "modules/timer.hpp"
#include "modules/window.hpp"
#include "util.hpp"

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

struct PlayerInputSystem {
    void update(float dt)
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

class RectangleRenderSystem {
public:
    RectangleRenderSystem()
        : shapes_(myl::getComponentId("RectangleRender"))
    {
    }

    void update(float dt)
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

class DrawFpsSystem {
public:
    DrawFpsSystem()
        : frameCounter_(0)
        , nextCalcFps_(timer::getTime() + 1.0)
    {
        text_.setFont(getFont());
        text_.setCharacterSize(14);
        text_.setFillColor(sf::Color::White);
    }

    void update(float dt)
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
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) {
        std::cerr << "Please pass a script file" << std::endl;
        return 1;
    }

    myl::loadComponentsFromFile("components.toml");

    myl::registerComponent("Transform",
        myl::StructBuilder()
            .addField("position", &Transform::position)
            .addField("angle", &Transform::angle)
            .build());

    PlayerInputSystem playerInput;
    myl::registerSystem("PlayerInput", [&](float dt) { playerInput.update(dt); });

    RectangleRenderSystem rectangleRender;
    myl::registerSystem("RectangleRender", [&](float dt) { rectangleRender.update(dt); });

    DrawFpsSystem drawFpsSystem;
    myl::registerSystem("DrawFps", [&](float dt) { drawFpsSystem.update(dt); });

    DebugSystem debugSystem;
    myl::registerSystem("_Debug", [&](float dt) { debugSystem.update(dt); });

    sol::state lua;
    myl::Lua::init(lua);

    std::cout << "Executing '" << args[0] << std::endl;
    lua.script_file(args[0]);

    std::cout << "Executing main()" << std::endl;
    const auto result = lua["myl"]["main"]();
    if (result.valid()) {
        std::cout << "Mainloop exited." << std::endl;
    } else {
        const sol::error err = result;
        std::cerr << "Error: " << err.what() << std::endl;
    }

    // We would segfault upon returning without this, because the systems store update
    // functions (lambdas) that reference the lua state, while
    // lua state itself references the world in a number of places too
    // (Circular references - great design).
    // TODO: Fix this properly. Currently this is sort of a hack.
    myl::getSystems().clear();

    return 0;
}