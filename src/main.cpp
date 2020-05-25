#include <cmath>

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <imgui.h>

#include "ecs.hpp"
#include "lua/lua.hpp"
#include "modules/input.hpp"
#include "modules/timer.hpp"
#include "modules/window.hpp"

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
            const auto lr = floatKey(input::Key::right) - floatKey(input::Key::left);
            const auto ud = floatKey(input::Key::down) - floatKey(input::Key::up);
            const auto moveDir = glm::vec2(lr, ud);
            input->moveDir = moveDir / (glm::length(moveDir) + 1.0e-9f);
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
            window::getWindow().draw(shape);
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
    World& world_;
    sf::Text text_;
    size_t frameCounter_;
    double nextCalcFps_;
};

class EntityInspectorSystem {
public:
    EntityInspectorSystem(World& world)
        : world_(world)
        , selectedEntity_(maxEntityId)
    {
        text_.setFont(getFont());
        text_.setCharacterSize(12);
        text_.setFillColor(sf::Color::White);
        text_.setOrigin(0.0f, -50.0f);
    }

    void update(float dt)
    {
        const auto entities = world_.getEntities();

        ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Entity Inspector");

        // Entity List
        {
            ImGui::BeginChild("entity list", ImVec2(200, 0), true);
            for (auto entity : entities) {
                const auto title = "Entity " + std::to_string(entity);
                if (ImGui::Selectable(title.c_str(), selectedEntity_ == entity))
                    selectedEntity_ = entity;
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Component Inspector
        if (world_.entityExists(selectedEntity_)) {
            const auto entity = selectedEntity_;
            ImGui::BeginChild("components", ImVec2(0, 0), true);
            for (const auto& component : world_.getComponents()) {
                if (world_.hasComponent(entity, component.getId())) {
                    auto ptr = world_.getComponent(entity, component.getId());
                    if (ImGui::CollapsingHeader(getComponentCaption(component, ptr).c_str(),
                            ImGuiTreeNodeFlags_DefaultOpen)) {
                        showComponentElements(component, ptr);
                    }
                }
            }

            ImGui::EndChild();
        }

        ImGui::End();

        // ImGui::ShowDemoWindow(nullptr);
    }

private:
    static std::string getComponentCaption(const Component& component, const void* ptr)
    {
        std::stringstream ss;
        ss << component.getId() << ": " << component.getName() << " (0x" << ptr << ")";
        return ss.str();
    }

    static void showFieldElement(const Struct::Field& field, void* ptr)
    {
        if (field.type->fieldType == FieldType::builtin) {
            auto ft = std::dynamic_pointer_cast<BuiltinFieldType>(field.type);
            switch (ft->type) {
            case BuiltinFieldType::f32:
                ImGui::InputFloat(field.name.c_str(), reinterpret_cast<float*>(ptr));
                break;
            case BuiltinFieldType::vec2:
                ImGui::InputFloat2(field.name.c_str(), reinterpret_cast<float*>(ptr));
                break;
            default:
                ImGui::Text("Unimplemented Builtin Type");
            }
        } else {
            ImGui::Text("Unimplemented Field Type");
        }
    }

    static void showComponentElements(const Component& component, void* ptr)
    {
        ImGui::LabelText("field", "value");
        for (const auto& field : component.getStruct().getFields()) {
            auto fieldPtr = reinterpret_cast<uint8_t*>(ptr) + field.offset;
            showFieldElement(field, fieldPtr);
        }
    }

    World& world_;
    sf::Text text_;
    EntityId selectedEntity_;
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

    EntityInspectorSystem entityInspectorSystem(world);
    world.registerSystem("EntityInspector", [&](float dt) { entityInspectorSystem.update(dt); });

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