#include "lua.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>

#include "../modules/input.hpp"
#include "../modules/timer.hpp"
#include "../modules/tweak.hpp"
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

static const char vec3lua[] =
#include "vec3.lua"
;

static const char vec4lua[] =
#include "vec4.lua"
;

static const char colorlua[] =
#include "color.lua"
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
        case PrimitiveFieldType::Type::color:
            return "color";
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
        auto timer = lua["myl"]["service"]["timer"] = lua.create_table();
        timer["getTime"] = myl::modules::timer::getTime;
        timer["getDelta"] = myl::modules::timer::getDelta;
    }

    void addInputModule(sol::state& lua)
    {
        auto input = lua["myl"]["service"]["input"] = lua.create_table();
        input["getKeyboardDown"]
            = static_cast<bool (*)(const std::string&)>(myl::modules::input::getKeyboardDown);
        input["getKeyboardPressed"]
            = static_cast<bool (*)(const std::string&)>(myl::modules::input::getKeyboardPressed);
        input["getKeyboardReleased"]
            = static_cast<bool (*)(const std::string&)>(myl::modules::input::getKeyboardReleased);
    }

    void addTweakModule(sol::state& lua)
    {
        using namespace myl::modules::tweak;
        auto tweak = lua["myl"]["service"]["tweak"] = lua.create_table();
        tweak["getInt"] = sol::overload(static_cast<int (*)(const std::string& name)>(get<int>),
            static_cast<int (*)(const std::string&, const int&)>(get<int>));
        tweak["getFloat"]
            = sol::overload(static_cast<float (*)(const std::string& name)>(get<float>),
                static_cast<float (*)(const std::string&, const float&)>(get<float>));
        tweak["getString"] = sol::overload(
            static_cast<std::string (*)(const std::string& name)>(get<std::string>),
            static_cast<std::string (*)(const std::string&, const std::string&)>(get<std::string>));

        // yaaaaaasss. copy&paste! I'm such good coder :):):):)

        tweak["_getVec2"] = sol::overload(
            [](const std::string& name) -> std::tuple<float, float> {
                const auto v = get<glm::vec2>(name);
                return std::make_tuple(v.x, v.y);
            },
            [](const std::string& name, float x, float y) -> std::tuple<float, float> {
                const auto v = get(name, glm::vec2(x, y));
                return std::make_tuple(v.x, v.y);
            });
        lua.script(R"(
            local _getVec2 = myl.service.tweak._getVec2
            function myl.service.tweak.getVec2(name, vec)
                if vec then
                    return myl.vec2(_getVec2(name, vec.x, vec.y))
                else
                    return myl.vec2(_getVec2(name))
                end
            end)");

        tweak["_getVec3"] = sol::overload(
            [](const std::string& name) -> std::tuple<float, float, float> {
                const auto v = get<glm::vec3>(name);
                return std::make_tuple(v.x, v.y, v.z);
            },
            [](const std::string& name, float x, float y,
                float z) -> std::tuple<float, float, float> {
                const auto v = get(name, glm::vec3(x, y, z));
                return std::make_tuple(v.x, v.y, v.z);
            });
        lua.script(R"(
            local _getVec3 = myl.service.tweak._getVec3
            function myl.service.tweak.getVec3(name, vec)
                if vec then
                    return myl.vec3(_getVec3(name, vec.x, vec.y, vec.z))
                else
                    return myl.vec3(_getVec3(name))
                end
            end)");

        tweak["_getVec4"] = sol::overload(
            [](const std::string& name) -> std::tuple<float, float, float, float> {
                const auto v = get<glm::vec4>(name);
                return std::make_tuple(v.x, v.y, v.z, v.w);
            },
            [](const std::string& name, float x, float y, float z,
                float w) -> std::tuple<float, float, float, float> {
                const auto v = get(name, glm::vec4(x, y, z, w));
                return std::make_tuple(v.x, v.y, v.z, v.w);
            });
        lua.script(R"(
            local _getVec4 = myl.service.tweak._getVec4
            function myl.service.tweak.getVec4(name, vec)
                if vec then
                    return myl.vec4(_getVec4(name, vec.x, vec.y, vec.z, vec.w))
                else
                    return myl.vec4(_getVec4(name))
                end
            end)");

        tweak["_getColor"] = sol::overload(
            [](const std::string& name) -> std::tuple<float, float, float, float> {
                const auto v = get<myl::Color>(name);
                return std::make_tuple(v.r, v.g, v.b, v.a);
            },
            [](const std::string& name, float r, float g, float b,
                float a) -> std::tuple<float, float, float, float> {
                const auto v = get(name, myl::Color(r, g, b, a));
                return std::make_tuple(v.r, v.g, v.b, v.a);
            });
        lua.script(R"(
            local _getColor = myl.service.tweak._getColor

            function myl.service.tweak.getColor(name, col)
                if col then
                    return myl.color(_getColor(name, col.r, col.g, col.b, col.a))
                else
                    return myl.color(_getColor(name))
                end
            end)");

        tweak["_setInt"] = set<int>;
        tweak["_setFloat"] = set<float>;
        tweak["_setVec2"]
            = [](const std::string& name, float x, float y) { set(name, glm::vec2(x, y)); };
        tweak["_setVec3"] = [](const std::string& name, float x, float y, float z) {
            set(name, glm::vec3(x, y, z));
        };
        tweak["_setVec4"] = [](const std::string& name, float x, float y, float z, float w) {
            set(name, glm::vec4(x, y, z, w));
        };
        tweak["_setColor"] = [](const std::string& name, float r, float g, float b, float a) {
            set(name, myl::Color(r, g, b, a));
        };

        lua.script(R"(
            local _setInt = myl.service.tweak._setInt
            local _setFloat = myl.service.tweak._setFloat
            local _setVec2 = myl.service.tweak._setVec2
            local _setVec3 = myl.service.tweak._setVec3
            local _setVec4 = myl.service.tweak._setVec4
            local _setColor = myl.service.tweak._setColor

            function myl.service.tweak.set(name, val)
                if type(val) == "number" then
                    _setFloat(name, val)
                elseif type(val) == "cdata" then
                    if ffi.istype("vec2", val) then
                        _setVec2(name, val.x, val.y)
                    elseif ffi.istype("vec3", val) then
                        _setVec3(name, val.x, val.y, val.z)
                    elseif fii.istype("vec4", val) then
                        _setVec4(name, val.x, val.y, val.z, val.w)
                    elseif ffi.istype("color", val) then
                        _setColor(name, val.r, val.g, val.b, val.a)
                    end
                end
            end
        )");

        tweak["save"] = myl::modules::tweak::save;
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
                [entities, index](sol::this_state /*L*/) mutable -> std::optional<EntityId> {
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

        lua_.script(liblua);
        lua_.script(mylstring);
        lua_.script(vec2lua);
        lua_.script(vec3lua);
        lua_.script(vec4lua);
        lua_.script(colorlua);

        myl["service"] = lua_.create_table();
        addWindowModule(lua_);
        addTimerModule(lua_);
        addInputModule(lua_);
        addTweakModule(lua_);

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

    int State::exceptionHandler(lua_State* L,
        sol::optional<const std::exception&> /*maybeException*/, sol::string_view description)
    {
        std::cerr << "Exception: " << description << std::endl;
        return sol::stack::push(L, description);
    }
}

}
