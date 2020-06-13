#include "fieldtype.hpp"

#include <cassert>
#include <sstream>

#include "color.hpp"
#include "structstring.hpp"
#include "structvector.hpp"

namespace myl {

FieldType::FieldType(Type fieldType)
    : fieldType(fieldType)
{
}

void FieldType::init(void* /*ptr*/) const
{
    // do nothing most of the time
}

void FieldType::free(void* /*ptr*/) const
{
    // do nothing most of the time
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
    : FieldType(FieldType::Error)
    , typeName(typeName)
{
}

std::string ErrorFieldType::asString() const
{
    return "error type (" + typeName + ")";
}

const std::map<std::string, PrimitiveFieldType::Type> PrimitiveFieldType::typeFromString {
    { "invalid", Invalid },
    { "bool", Bool },
    { "u8", U8 },
    { "i8", I8 },
    { "u16", U16 },
    { "i16", I16 },
    { "u32", U32 },
    { "i32", I32 },
    { "u64", U64 },
    { "i64", I64 },
    { "f32", F32 },
    { "vec2", Vec2 },
    { "vec3", Vec3 },
    { "vec4", Vec4 },
    { "color", Color },
};

PrimitiveFieldType::PrimitiveFieldType(Type type)
    : FieldType(FieldType::Builtin)
    , type(type)
{
}

std::string PrimitiveFieldType::asString() const
{
    switch (type) {
    case Type::Invalid:
        return "invalid";
    case Type::Bool:
        return "bool";
    case Type::U8:
        return "u8";
    case Type::I8:
        return "i8";
    case Type::U16:
        return "u16";
    case Type::I16:
        return "i16";
    case Type::U32:
        return "u32";
    case Type::I32:
        return "i32";
    case Type::U64:
        return "u64";
    case Type::I64:
        return "i64";
    case Type::F32:
        return "f32";
    case Type::Vec2:
        return "vec2";
    case Type::Vec3:
        return "vec3";
    case Type::Vec4:
        return "vec4";
    case Type::Color:
        return "color";
    default:
        return "unknown";
    };
}

size_t PrimitiveFieldType::getSize() const
{
    switch (type) {
    case Type::Invalid:
        assert(false && "type = invalid");
    case Type::Bool:
        return sizeof(bool);
    case Type::U8:
        return sizeof(uint8_t);
    case Type::I8:
        return sizeof(int8_t);
    case Type::U16:
        return sizeof(uint16_t);
    case Type::I16:
        return sizeof(int16_t);
    case Type::U32:
        return sizeof(uint32_t);
    case Type::I32:
        return sizeof(int32_t);
    case Type::U64:
        return sizeof(uint64_t);
    case Type::I64:
        return sizeof(int64_t);
    case Type::F32:
        return sizeof(float);
    case Type::Vec2:
        return sizeof(float[2]);
    case Type::Vec3:
        return sizeof(float[3]);
    case Type::Vec4:
        return sizeof(float[4]);
    case Type::Color:
        return sizeof(Color);
    default:
        return 0;
    };
}

size_t PrimitiveFieldType::getAlignment() const
{
    switch (type) {
    case Type::Invalid:
        assert(false && "type = invalid");
    case Type::Bool:
        return std::alignment_of_v<bool>;
    case Type::U8:
        return std::alignment_of_v<uint8_t>;
    case Type::I8:
        return std::alignment_of_v<int8_t>;
    case Type::U16:
        return std::alignment_of_v<uint16_t>;
    case Type::I16:
        return std::alignment_of_v<int16_t>;
    case Type::U32:
        return std::alignment_of_v<uint32_t>;
    case Type::I32:
        return std::alignment_of_v<int32_t>;
    case Type::U64:
        return std::alignment_of_v<uint64_t>;
    case Type::I64:
        return std::alignment_of_v<int64_t>;
    case Type::F32:
        return std::alignment_of_v<float>;
    case Type::Vec2:
        return std::alignment_of_v<float[2]>;
    case Type::Vec3:
        return std::alignment_of_v<float[3]>;
    case Type::Vec4:
        return std::alignment_of_v<float[4]>;
    case Type::Color:
        return std::alignment_of_v<myl::Color>;
    default:
        return 0;
    };
}

StringFieldType::StringFieldType()
    : FieldType(FieldType::String)
{
}

void StringFieldType::free(void* ptr) const
{
    reinterpret_cast<myl::String*>(ptr)->~String();
}

std::string StringFieldType::asString() const
{
    return "string";
}

size_t StringFieldType::getSize() const
{
    return sizeof(String);
}

size_t StringFieldType::getAlignment() const
{
    return std::alignment_of_v<myl::String>;
}

EnumFieldType::EnumFieldType(const std::string& name)
    : FieldType(FieldType::Enum)
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
    : FieldType(FieldType::Array)
    , elementType(std::move(elementType))
    , size(size)
{
}

void ArrayFieldType::init(void* ptr) const
{
    for (size_t i = 0; i < size; ++i)
        elementType->init(reinterpret_cast<uint8_t*>(ptr) + i * elementType->getSize());
}

void ArrayFieldType::free(void* ptr) const
{
    for (size_t i = 0; i < size; ++i)
        elementType->free(reinterpret_cast<uint8_t*>(ptr) + i * elementType->getSize());
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
    : FieldType(FieldType::Vector)
    , elementType(std::move(elementType))
{
}

size_t VectorFieldType::getSize() const
{
    return sizeof(myl::Vector);
}

size_t VectorFieldType::getAlignment() const
{
    return std::alignment_of_v<myl::Vector>;
}

void VectorFieldType::init(void* ptr) const
{
    new (ptr) myl::Vector(elementType.get());
}

void VectorFieldType::free(void* ptr) const
{
    reinterpret_cast<myl::Vector*>(ptr)->~Vector();
}

std::string VectorFieldType::asString() const
{
    return "vector<" + elementType->asString() + ">";
}

MapFieldType::MapFieldType(std::shared_ptr<FieldType> keyType, std::shared_ptr<FieldType> valueType)
    : FieldType(FieldType::Map)
    , keyType(std::move(keyType))
    , valueType(std::move(valueType))
{
}

void MapFieldType::init(void* /*ptr*/) const
{
    assert(false && "Unimplemented: map init");
}

void MapFieldType::free(void* /*ptr*/) const
{
    assert(false && "Unimplemented: map free");
}

std::string MapFieldType::asString() const
{
    return "map<" + keyType->asString() + ", " + valueType->asString() + ">";
}

StructFieldType::StructFieldType(const std::string& name)
    : FieldType(FieldType::Struct)
    , name(name)
{
}

void StructFieldType::init(void* /*ptr*/) const
{
    assert(false && "Unimplemented: struct init");
}

void StructFieldType::free(void* /*ptr*/) const
{
    assert(false && "Unimplemented: struct free");
}

std::string StructFieldType::asString() const
{
    return "struct " + name;
}

EnumType::EnumType(const std::vector<std::string>& valueNames)
    : underlyingType(std::make_shared<PrimitiveFieldType>(PrimitiveFieldType::I32))
{
    for (size_t i = 0; i < valueNames.size(); ++i) {
        values.emplace_back(valueNames[i], static_cast<int64_t>(i));
    }
}

std::string EnumType::asString() const
{
    std::stringstream ss;
    ss << "{";
    for (size_t i = 0; i < values.size(); ++i) {
        ss << values[i].first;
        if (values[i].second != static_cast<int64_t>(i))
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
