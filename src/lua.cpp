#include "lua.hpp"

#include <cassert>

static const auto lib = R"(
local ffi = require("ffi")

function myl.getComponent(component, entityId)
    return ffi.cast(component .. "*", myl._getComponent(component, entityId))[0]
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
        assert(false && "vec2 unimplemented");
    case BuiltinFieldType::Type::vec3:
        assert(false && "vec3 unimplemented");
    case BuiltinFieldType::Type::vec4:
        assert(false && "vec4 unimplemented");
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

    lua.create_named_table("myl");
    lua["myl"]["_getComponent"].set_function(
        [&world](const std::string& name, EntityId entityId) -> sol::lightuserdata_value {
            return world.getComponent(name, entityId);
        });
    lua.script(lib);

    for (const auto& [name, structData] : componentData.structs) {
        const auto code = Lua::getAsCString(structData);
        lua["ffi"]["cdef"](code);
    }
}
}