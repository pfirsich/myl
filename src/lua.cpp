#include "lua.hpp"

#include <cassert>

static const auto lib = R"(
local ffi = require("ffi")

ffi.cdef [[
typedef struct {
    float x, y;
} vec2;

typedef struct {
    float x, y, z;
} vec3;

typedef struct {
    float x, y, z, w;
} vec4;
]]

function myl.addComponent(entityId, component)
    return ffi.cast(component .. "*", myl._addComponent(entityId, component))[0]
end

function myl.getComponent(entityId, component)
    return ffi.cast(component .. "*", myl._getComponent(entityId, component))[0]
end

myl.phases = {
    init = 0,
    postInit = 1,
    preUpdate = 2,
    update = 3,
    postUpdate = 4,
    preDraw = 5,
    draw = 6,
    postDraw = 7,

    addComponent = 8,
    removeComponent = 9,
}

function myl.registerSystem(name, phase, requiresComponents, func)
end
)";

namespace Lua {

std::string getCTypeName(BuiltinFieldType::Type type)
{
    switch (type) {
    case BuiltinFieldType::Type::invalid:
        assert(false && "Invaid BuiltinFieldType");
    case BuiltinFieldType::Type::bool_:
        return "bool";
    case BuiltinFieldType::Type::u8:
        return "uint8_t";
    case BuiltinFieldType::Type::i8:
        return "int8_t";
    case BuiltinFieldType::Type::u16:
        return "uint16_t";
    case BuiltinFieldType::Type::i16:
        return "int16_t";
    case BuiltinFieldType::Type::u32:
        return "uint32_t";
    case BuiltinFieldType::Type::i32:
        return "int32_t";
    case BuiltinFieldType::Type::u64:
        return "uint64_t";
    case BuiltinFieldType::Type::i64:
        return "int64_t";
    case BuiltinFieldType::Type::f32:
        return "float";
    case BuiltinFieldType::Type::vec2:
        return "vec2";
    case BuiltinFieldType::Type::vec3:
        return "vec3";
    case BuiltinFieldType::Type::vec4:
        return "vec4";
    case BuiltinFieldType::Type::string:
        assert(false && "string unimplemented");
    };
}

std::string getCTypeName(std::shared_ptr<FieldType> fieldType)
{
    return visit<std::string>(
        [](auto&& arg) -> std::string {
            using T = typename std::decay_t<decltype(arg)>::element_type;
            if constexpr (std::is_same_v<T, ErrorFieldType>)
                assert(false && "ErrorFieldType");
            else if constexpr (std::is_same_v<T, BuiltinFieldType>)
                return getCTypeName(arg->type);
            else if constexpr (std::is_same_v<T, EnumFieldType>)
                return arg->name;
            else if constexpr (std::is_same_v<T, StructFieldType>)
                return arg->name;
            else if constexpr (std::is_same_v<T, ArrayFieldType>)
                return getCTypeName(arg->elementType) + "[" + std::to_string(arg->size) + "]";
            else if constexpr (std::is_same_v<T, VectorFieldType>)
                assert(false && "VectorFieldType unimplemented");
            else if constexpr (std::is_same_v<T, MapFieldType>)
                assert(false && "MapFieldType unimplemented");
        },
        fieldType);
}

std::string getAsCString(const StructData& structData)
{
    std::stringstream ss;
    ss << "typedef struct {\n";
    for (const auto& [fieldName, fieldType] : structData.structType.fields) {
        ss << "    " << getCTypeName(fieldType) << " " << fieldName << ";\n";
    }
    ss << "} " << structData.name;
    return ss.str();
    return "";
}

void init(sol::state& lua, const ComponentFileData& componentData, World& world)
{
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::ffi);

    auto myl = lua.create_named_table("myl");
    myl["entityExists"].set_function(
        [&world](EntityId entityId) -> bool { return world.entityExists(entityId); });
    myl["newEntity"].set_function([&world]() -> EntityId { return world.newEntity(); });
    myl["destroyEntity"].set_function(
        [&world](EntityId entityId) -> void { world.destroyEntity(entityId); });
    myl["removeComponent"].set_function(
        [&world](EntityId entityId, const std::string& name) -> void {
            world.removeComponent(entityId, name);
        });
    myl["_addComponent"].set_function(
        [&world](EntityId entityId, const std::string& name) -> sol::lightuserdata_value {
            return world.addComponent(entityId, name);
        });
    myl["_getComponent"].set_function(
        [&world](EntityId entityId, const std::string& name) -> sol::lightuserdata_value {
            return world.getComponent(entityId, name);
        });
    lua.script(lib);

    for (const auto& [name, structData] : componentData.structs) {
        const auto code = Lua::getAsCString(structData);
        lua["ffi"]["cdef"](code);
    }
}
}