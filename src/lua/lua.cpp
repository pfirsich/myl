#include "lua.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

#include "../modules/input.hpp"
#include "../modules/timer.hpp"
#include "../modules/window.hpp"

namespace fs = std::filesystem;

namespace myl {

namespace lua {

    // clang-format off

static const char liblua[] =
#include "lib.lua"
;

static const char mylstring[] =
#include "string.lua"
;

static const char vec2lua[] =
#include "vec2.lua"
;

    // clang-format on

    std::string getCTypeName(PrimitiveFieldType::Type type)
    {
        switch (type) {
        case PrimitiveFieldType::Type::invalid:
            assert(false && "Invalid BuiltinFieldType");
        case PrimitiveFieldType::Type::bool_:
            return "bool";
        case PrimitiveFieldType::Type::u8:
            return "uint8_t";
        case PrimitiveFieldType::Type::i8:
            return "int8_t";
        case PrimitiveFieldType::Type::u16:
            return "uint16_t";
        case PrimitiveFieldType::Type::i16:
            return "int16_t";
        case PrimitiveFieldType::Type::u32:
            return "uint32_t";
        case PrimitiveFieldType::Type::i32:
            return "int32_t";
        case PrimitiveFieldType::Type::u64:
            return "uint64_t";
        case PrimitiveFieldType::Type::i64:
            return "int64_t";
        case PrimitiveFieldType::Type::f32:
            return "float";
        case PrimitiveFieldType::Type::vec2:
            return "vec2";
        case PrimitiveFieldType::Type::vec3:
            return "vec3";
        case PrimitiveFieldType::Type::vec4:
            return "vec4";
        default:
            assert(false && "Unknown BuiltinFieldType");
        };
    }

    std::string getCTypeName(std::shared_ptr<FieldType> fieldType)
    {
        return visit<std::string>(
            [](auto&& arg) -> std::string {
                using T = typename std::decay_t<decltype(arg)>::element_type;
                if constexpr (std::is_same_v<T, ErrorFieldType>)
                    assert(false && "ErrorFieldType");
                else if constexpr (std::is_same_v<T, PrimitiveFieldType>)
                    return getCTypeName(arg->type);
                else if constexpr (std::is_same_v<T, StringFieldType>)
                    return "MylString";
                else if constexpr (std::is_same_v<T, EnumFieldType>)
                    return arg->name;
                else if constexpr (std::is_same_v<T, StructFieldType>)
                    return arg->name;
                else if constexpr (std::is_same_v<T, ArrayFieldType>)
                    return getCTypeName(arg->elementType) + "[" + std::to_string(arg->size) + "]";
                else if constexpr (std::is_same_v<T, VectorFieldType>)
                    return "";
                // return "MylVector_" + )
                // assert(false && "VectorFieldType unimplemented");
                else if constexpr (std::is_same_v<T, MapFieldType>)
                    assert(false && "MapFieldType unimplemented");
            },
            fieldType);
    }

    std::string getAsCString(const Component& component)
    {
        std::stringstream ss;
        ss << "typedef struct {\n";
        for (const auto& field : component.getStruct().getFields()) {
            ss << "    " << getCTypeName(field.type) << " " << field.name << ";\n";
        }
        ss << "} " << component.getName();
        return ss.str();
        return "";
    }

    void addWindowModule(sol::state& lua)
    {
        std::cout << "Init service 'window'" << std::endl;
        auto window = lua["myl"]["service"]["window"] = lua.create_table();
        window["init"] = myl::modules::window::init;
        window["setTitle"] = myl::modules::window::setTitle;
        window["setVSync"] = myl::modules::window::setVSync;
        window["update"] = myl::modules::window::update;
        window["clear"] = myl::modules::window::clear;
        window["present"] = myl::modules::window::present;
    }

    void addTimerModule(sol::state& lua)
    {
        std::cout << "Init service 'timer'" << std::endl;
        auto timer = lua["myl"]["service"]["timer"] = lua.create_table();
        timer["getTime"] = myl::modules::timer::getTime;
        timer["getDelta"] = myl::modules::timer::getDelta;
    }

    void addInputModule(sol::state& lua)
    {
        std::cout << "Init service 'input'" << std::endl;
        auto input = lua["myl"]["service"]["input"] = lua.create_table();
        input["getKeyboardDown"]
            = static_cast<bool (*)(const std::string&)>(myl::modules::input::getKeyboardDown);
        input["getKeyboardPressed"]
            = static_cast<bool (*)(const std::string&)>(myl::modules::input::getKeyboardPressed);
        input["getKeyboardReleased"]
            = static_cast<bool (*)(const std::string&)>(myl::modules::input::getKeyboardReleased);
    }

    State::State(myl::World& world)
        : world_(world)
    {
        assert(&world == &getDefaultWorld() && "Unimplemented non-default world");
    }

    State::State()
        : State(getDefaultWorld())
    {
    }

    State::~State()
    {
        for (const auto& name : registeredSystems_)
            world_.unregisterSystem(name);
    }

    void State::init()
    {
        lua_.open_libraries(sol::lib::base, sol::lib::package, sol::lib::coroutine,
            sol::lib::string, sol::lib::os, sol::lib::math, sol::lib::table, sol::lib::bit32,
            sol::lib::io, sol::lib::ffi, sol::lib::jit, sol::lib::utf8);
        lua_.set_exception_handler(&exceptionHandler);

        auto myl = lua_.create_named_table("myl");

        myl["entityExists"].set_function(entityExists);
        myl["newEntity"].set_function(newEntity);
        myl["destroyEntity"].set_function(destroyEntity);

        myl["foreachEntity"].set_function([](sol::variadic_args va) {
            ComponentMask mask;
            for (auto v : va)
                mask += ComponentId(v.as<size_t>());
            const auto entities = getEntities(mask);

            size_t index = 0;
            return sol::as_function(
                [entities, index](sol::this_state L) mutable -> std::optional<EntityId> {
                    if (index < entities.size())
                        return entities[index++];
                    return std::nullopt;
                });
        });

        myl["removeComponent"].set_function(removeComponent);
        myl["hasComponent"].set_function(hasComponent);

        myl["_addComponent"].set_function(
            [](EntityId entityId, size_t compId) -> sol::lightuserdata_value {
                return addComponent(entityId, ComponentId(compId));
            });
        myl["_getComponent"].set_function(
            [](EntityId entityId, size_t compId) -> sol::lightuserdata_value {
                return getComponent(entityId, ComponentId(compId));
            });

        myl["setComponentEnabled"].set_function(sol::overload(
            [](EntityId id, ComponentId compId) -> void {
                return setComponentEnabled(id, compId, true);
            },
            [](EntityId id, ComponentId compId, bool enabled) -> void {
                return setComponentEnabled(id, compId, enabled);
            }));

        myl["setComponentDisabled"].set_function(setComponentDisabled);

        myl["setSystemEnabled"].set_function(sol::overload(
            [](const std::string& name) -> void { return setSystemEnabled(name, true); },
            [](const std::string& name, bool enabled) -> void {
                return setSystemEnabled(name, enabled);
            }));

        myl["setSystemDisabled"].set_function(setSystemDisabled);

        myl["registerSystem"].set_function([this](const std::string& name, sol::function function) {
            registerSystem(name, [function](float dt) {
                const auto result = function(dt);
                if (!result.valid()) {
                    const sol::error err = result;
                    std::cerr << "Error: " << err.what() << std::endl;
                    assert(false);
                }
            });
            registeredSystems_.emplace_back(name);
        });
        myl["invokeSystem"].set_function(invokeSystem);

        myl["loadComponents"].set_function(
            static_cast<void (*)(const std::string&)>(myl::loadComponents));

        std::cout << "Load lib" << std::endl;
        lua_.script(liblua);
        lua_.script(mylstring);
        lua_.script(vec2lua);

        std::cout << "Init services" << std::endl;
        myl["service"] = lua_.create_table();
        addWindowModule(lua_);
        addTimerModule(lua_);
        addInputModule(lua_);

        connection_ = getDefaultWorld().componentRegistered.connect(
            [this](const Component& component) { componentRegistered(lua_, component); });

        myl["c"] = lua_.create_table();
        myl["_componentTypes"] = lua_.create_table();
        for (const auto& component : getComponents())
            componentRegistered(lua_, component);

        if (fs::exists("main.lua"))
            lua_.script_file("main.lua");
    }

    bool State::hasMain() const
    {
        return lua_["myl"]["main"].valid();
    }

    bool State::executeMain()
    {
        const auto result = lua_["myl"]["main"]();
        if (!result.valid()) {
            const sol::error err = result;
            std::cerr << "Error excecuting main: " << err.what() << std::endl;
            return false;
        }
        return true;
    }

    void State::componentRegistered(sol::state& lua, const Component& component)
    {
        lua["ffi"]["cdef"](getAsCString(component));
        const auto& name = component.getName();
        const auto id = static_cast<size_t>(getComponentId(name));
        lua["myl"]["c"][name] = id;
        lua["myl"]["_componentTypes"][id] = name + "*";
    }

    int State::exceptionHandler(lua_State* L, sol::optional<const std::exception&> maybeException,
        sol::string_view description)
    {
        std::cerr << "Exception: " << description << std::endl;
        return sol::stack::push(L, description);
    }
}

}
