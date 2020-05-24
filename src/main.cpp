#include <cmath>

#include <SFML/Graphics.hpp>

#include "componentfile.hpp"
#include "ecs.hpp"
#include "lua.hpp"
#include "modules/window.hpp"
#include "struct.hpp"

struct vec2 {
    float x, y;

    float length() const
    {
        return std::sqrt(x * x + y * y);
    }
};

struct Transform {
    vec2 position;
    float angle;
};

struct Velocity {
    vec2 value;
};

struct PlayerInputState {
    vec2 moveDir;
};

struct RectangleRender {
    vec2 size;
};

template <typename T>
constexpr T convert(bool v, const T& falseVal, const T& trueVal)
{
    return v ? falseVal : trueVal;
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
            const auto lr
                = convert<float>(sf::Keyboard::isKeyPressed(sf::Keyboard::Right), 0.0f, 1.0f)
                - convert<float>(sf::Keyboard::isKeyPressed(sf::Keyboard::Left), 0.0f, 1.0f);
            const auto ud
                = convert<float>(sf::Keyboard::isKeyPressed(sf::Keyboard::Down), 0.0f, 1.0f)
                - convert<float>(sf::Keyboard::isKeyPressed(sf::Keyboard::Up), 0.0f, 1.0f);
            const auto len = vec2 { lr, ud }.length();
            input->moveDir.x = lr / len;
            input->moveDir.y = ud / len;
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

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) {
        std::cerr << "Please pass a script file" << std::endl;
        return 1;
    }

    const auto componentData = loadComponentFromFile("components.toml");
    std::vector<Component> components;
    for (const auto& [name, component] : componentData.structs) {
        if (!component.isComponent)
            continue;

        StructBuilder sb;
        for (const auto& [fieldName, fieldType] : component.structType.fields) {
            sb.addField(fieldName, fieldType);
        }
        components.emplace_back(name, sb.build());
    }

    World world(components);

    RectangleRenderSystem rectangleRender(world);
    world.registerSystem("RectangleRenderSystem", [&](float dt) { rectangleRender.update(dt); });

    sol::state lua;
    Lua::init(lua, componentData, world);

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