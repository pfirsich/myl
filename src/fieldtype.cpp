#include "fieldtype.hpp"

#include <cassert>
#include <sstream>

namespace myl {

FieldType::FieldType(Type fieldType)
    : fieldType(fieldType)
{
}

size_t FieldType::getSize() const
{
    assert(false && "Unimplemented");
    return 0;
}

size_t FieldType::getAlignment() const
{
    assert(false && "Unimplemented");
    return 0;
}

ErrorFieldType::ErrorFieldType(const std::string& typeName)
    : FieldType(FieldType::error)
    , typeName(typeName)
{
}

std::string ErrorFieldType::asString() const
{
    return "error type (" + typeName + ")";
}

const std::map<std::string, BuiltinFieldType::Type> BuiltinFieldType::typeFromString {
    { "invalid", invalid },
    { "bool", bool_ },
    { "u8", u8 },
    { "i8", i8 },
    { "u16", u16 },
    { "i16", i16 },
    { "u32", u32 },
    { "i32", i32 },
    { "u64", u64 },
    { "i64", i64 },
    { "f32", f32 },
    { "vec2", vec2 },
    { "vec3", vec3 },
    { "vec4", vec4 },
    { "string", string },
};

BuiltinFieldType::BuiltinFieldType(Type type)
    : FieldType(FieldType::builtin)
    , type(type)
{
}

std::string BuiltinFieldType::asString() const
{
    switch (type) {
    case Type::invalid:
        return "invalid";
    case Type::bool_:
        return "bool";
    case Type::u8:
        return "u8";
    case Type::i8:
        return "i8";
    case Type::u16:
        return "u16";
    case Type::i16:
        return "i16";
    case Type::u32:
        return "u32";
    case Type::i32:
        return "i32";
    case Type::u64:
        return "u64";
    case Type::i64:
        return "i64";
    case Type::f32:
        return "f32";
    case Type::vec2:
        return "vec2";
    case Type::vec3:
        return "vec3";
    case Type::vec4:
        return "vec4";
    case Type::string:
        return "string";
    default:
        return "unknown";
    };
}

size_t BuiltinFieldType::getSize() const
{
    switch (type) {
    case Type::invalid:
        assert(false && "type = invalid");
    case Type::bool_:
        return sizeof(bool);
    case Type::u8:
        return sizeof(uint8_t);
    case Type::i8:
        return sizeof(int8_t);
    case Type::u16:
        return sizeof(uint16_t);
    case Type::i16:
        return sizeof(int16_t);
    case Type::u32:
        return sizeof(uint32_t);
    case Type::i32:
        return sizeof(int32_t);
    case Type::u64:
        return sizeof(uint64_t);
    case Type::i64:
        return sizeof(int64_t);
    case Type::f32:
        return sizeof(float);
    case Type::vec2:
        return sizeof(float[2]);
    case Type::vec3:
        return sizeof(float[3]);
    case Type::vec4:
        return sizeof(float[4]);
    case Type::string:
        return sizeof(String);
    default:
        return 0;
    };
}

size_t BuiltinFieldType::getAlignment() const
{
    switch (type) {
    case Type::invalid:
        assert(false && "type = invalid");
    case Type::bool_:
        return std::alignment_of_v<bool>;
    case Type::u8:
        return std::alignment_of_v<uint8_t>;
    case Type::i8:
        return std::alignment_of_v<int8_t>;
    case Type::u16:
        return std::alignment_of_v<uint16_t>;
    case Type::i16:
        return std::alignment_of_v<int16_t>;
    case Type::u32:
        return std::alignment_of_v<uint32_t>;
    case Type::i32:
        return std::alignment_of_v<int32_t>;
    case Type::u64:
        return std::alignment_of_v<uint64_t>;
    case Type::i64:
        return std::alignment_of_v<int64_t>;
    case Type::f32:
        return std::alignment_of_v<float>;
    case Type::vec2:
        return std::alignment_of_v<float[2]>;
    case Type::vec3:
        return std::alignment_of_v<float[3]>;
    case Type::vec4:
        return std::alignment_of_v<float[4]>;
    case Type::string:
        return std::alignment_of_v<String>;
    default:
        return 0;
    };
}

EnumFieldType::EnumFieldType(const std::string& name)
    : FieldType(FieldType::enum_)
    , name(name)
{
}

std::string EnumFieldType::asString() const
{
    return "enum " + name;
}

size_t EnumFieldType::getSize() const
{
    return sizeof(int);
}

size_t EnumFieldType::getAlignment() const
{
    return std::alignment_of_v<int>;
}

ArrayFieldType::ArrayFieldType(std::shared_ptr<FieldType> elementType, size_t size)
    : FieldType(FieldType::array)
    , elementType(std::move(elementType))
    , size(size)
{
}

std::string ArrayFieldType::asString() const
{
    return "array<" + elementType->asString() + ", " + std::to_string(size) + ">";
}

size_t ArrayFieldType::getSize() const
{
    return size * elementType->getSize();
}

size_t ArrayFieldType::getAlignment() const
{
    return elementType->getAlignment();
}

VectorFieldType::VectorFieldType(std::shared_ptr<FieldType> elementType)
    : FieldType(FieldType::vector)
    , elementType(std::move(elementType))
{
}

size_t VectorFieldType::getSize() const
{
    return sizeof(Vector);
}

size_t VectorFieldType::getAlignment() const
{
    return std::alignment_of_v<Vector>;
}

std::string VectorFieldType::asString() const
{
    return "vector<" + elementType->asString() + ">";
}

MapFieldType::MapFieldType(std::shared_ptr<FieldType> keyType, std::shared_ptr<FieldType> valueType)
    : FieldType(FieldType::map)
    , keyType(std::move(keyType))
    , valueType(std::move(valueType))
{
}

std::string MapFieldType::asString() const
{
    return "map<" + keyType->asString() + ", " + valueType->asString() + ">";
}

StructFieldType::StructFieldType(const std::string& name)
    : FieldType(FieldType::struct_)
    , name(name)
{
}

std::string StructFieldType::asString() const
{
    return "struct " + name;
}

EnumType::EnumType(const std::vector<std::string>& valueNames)
    : underlyingType(std::make_shared<BuiltinFieldType>(BuiltinFieldType::i32))
{
    for (int64_t i = 0; i < valueNames.size(); ++i) {
        values.emplace_back(std::pair<std::string, int64_t>(valueNames[i], i));
    }
}

std::string EnumType::asString() const
{
    std::stringstream ss;
    ss << "{";
    for (size_t i = 0; i < values.size(); ++i) {
        ss << values[i].first;
        if (values[i].second != i)
            ss << "(" << values[i].second << ")";
        if (i < values.size() - 1)
            ss << ", ";
    }
    ss << "}";
    return ss.str();
}

StructType::StructType()
{
}

std::string StructType::asString() const
{
    std::stringstream ss;
    ss << "{\n";
    for (size_t i = 0; i < fields.size(); ++i) {
        ss << "  " << fields[i].first << " : " << fields[i].second->asString();
        if (i < fields.size() - 1)
            ss << ",\n";
        else
            ss << "\n";
    }
    ss << "}";
    return ss.str();
}

}