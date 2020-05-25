#include <cmath>

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "componentfile.hpp"
#include "ecs.hpp"
#include "lua/lua.hpp"
#include "modules/timer.hpp"
#include "modules/window.hpp"
#include "struct.hpp"

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

float floatKey(sf::Keyboard::Key key)
{
    return sf::Keyboard::isKeyPressed(key) ? 1.0f : 0.0f;
}

class PlayerInputSystem {
public:
    PlayerInputSystem(World& world)
        : world_(world)
    {
    }

    void update(float dt)
    {
        const auto cPlayerInputState = world_.getComponentId("PlayerInputState");
        for (auto entity : world_.getEntities(cPlayerInputState)) {
            auto input = world_.getComponent<PlayerInputState>(entity, cPlayerInputState);
            const auto lr = floatKey(sf::Keyboard::Right) - floatKey(sf::Keyboard::Left);
            const auto ud = floatKey(sf::Keyboard::Down) - floatKey(sf::Keyboard::Up);
            const auto moveDir = glm::vec2(lr, ud);
            input->moveDir = moveDir / (moveDir.length() + 1.0e-9f);
        }
    }

private:
    World& world_;
};

class RectangleRenderSystem {
public:
    RectangleRenderSystem(World& world)
        : world_(world)
        , shapes_(world, world.getComponentId("RectangleRender"))
    {
    }

    void update(float dt)
    {
        const auto cTransform = world_.getComponentId("Transform");
        const auto cRectangleRender = world_.getComponentId("RectangleRender");
        const auto mask = ComponentMask(cTransform) + cRectangleRender;
        for (auto entity : world_.getEntities(mask)) {
            auto& shape = shapes_.get<true>(entity);
            auto trafo = world_.getComponent<Transform>(entity, cTransform);
            auto rectangleRender = world_.getComponent<RectangleRender>(entity, cRectangleRender);
            shape.setPosition(trafo->position.x, trafo->position.y);
            shape.setRotation(trafo->angle / M_PI * 180.0f);
            shape.setSize(sf::Vector2f(rectangleRender->size.x, rectangleRender->size.y));
            myl::modules::window::getWindow().draw(shape);
        }
        shapes_.remove();
    }

private:
    World& world_;
    SystemData<sf::RectangleShape> shapes_;
};

class DrawFpsSystem {
public:
    DrawFpsSystem(World& world)
        : world_(world)
        , frameCounter_(0)
        , nextCalcFps_(myl::modules::timer::getTime() + 1.0)
    {
        text_.setFont(getFont());
        text_.setCharacterSize(14);
        text_.setFillColor(sf::Color::White);
    }

    void update(float dt)
    {
        const auto now = myl::modules::timer::getTime();
        frameCounter_++;
        if (nextCalcFps_ < now) {
            nextCalcFps_ = now + 1.0;
            text_.setString("FPS: " + std::to_string(frameCounter_));
            frameCounter_ = 0;
        }
        myl::modules::window::getWindow().draw(text_);
    }

private:
    static sf::Font& getFont()
    {
        static sf::Font font;
        assert(font.loadFromFile("RobotoMono-Regular.ttf"));
        return font;
    }

    World& world_;
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

    World world;

    loadComponentsFromFile(world, "components.toml");

    world.registerComponent("Transform",
        StructBuilder()
            .addField("position", &Transform::position)
            .addField("angle", &Transform::angle)
            .build());

    PlayerInputSystem playerInput(world);
    world.registerSystem("PlayerInput", [&](float dt) { playerInput.update(dt); });

    RectangleRenderSystem rectangleRender(world);
    world.registerSystem("RectangleRender", [&](float dt) { rectangleRender.update(dt); });

    DrawFpsSystem drawFpsSystem(world);
    world.registerSystem("DrawFps", [&](float dt) { drawFpsSystem.update(dt); });

    sol::state lua;
    Lua::init(lua, world);

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

    return 0;
}