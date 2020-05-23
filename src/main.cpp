#include "componentfile.hpp"
#include "ecs.hpp"
#include "lua.hpp"
#include "struct.hpp"

struct MoveComponent {
    uint32_t id;
    bool flag;
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

    auto comp = world.addComponent<MoveComponent>(0, "MoveComponent");
    comp->id = 69;
    comp->flag = true;

    sol::state lua;
    Lua::init(lua, componentData, world);

    lua.script_file(args[0]);

    std::cout << (comp->flag ? "true" : "false") << std::endl;
}