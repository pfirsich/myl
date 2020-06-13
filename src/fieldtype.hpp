#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace myl {

struct FieldType {
    enum Type { Invalid, Error, Builtin, String, Enum, Struct, Array, Vector, Map } fieldType;

    FieldType(Type fieldType);
    virtual ~FieldType() = default;

    virtual void init(void* ptr) const;
    virtual void free(void* ptr) const;
    virtual std::string asString() const = 0;
    virtual size_t getSize() const;
    virtual size_t getAlignment() const;
};

struct ErrorFieldType : public FieldType {
    std::string typeName;

    ErrorFieldType(const std::string& typeName);

    std::string asString() const override;
};

struct PrimitiveFieldType : public FieldType {
    enum Type {
        Invalid,
        Bool,
        U8,
        I8,
        U16,
        I16,
        U32,
        I32,
        U64,
        I64,
        F32,
        Vec2,
        Vec3,
        Vec4,
        Color,
    } type;

    static const std::map<std::string, Type> typeFromString;

    PrimitiveFieldType(Type type);

    std::string asString() const override;
    size_t getSize() const override;
    size_t getAlignment() const override;
};

struct StringFieldType : public FieldType {
    StringFieldType();

    void free(void* ptr) const override;
    std::string asString() const override;
    size_t getSize() const override;
    size_t getAlignment() const override;
};

struct EnumFieldType : public FieldType {
    std::string name;

    EnumFieldType(const std::string& name);

    std::string asString() const override;
    size_t getSize() const override;
    size_t getAlignment() const override;
};

struct ArrayFieldType : public FieldType {
    std::shared_ptr<FieldType> elementType;
    size_t size;

    ArrayFieldType(std::shared_ptr<FieldType> elementType, size_t size);

    void init(void* ptr) const override;
    void free(void* ptr) const override;
    std::string asString() const override;
    size_t getSize() const override;
    size_t getAlignment() const override;
};

struct VectorFieldType : public FieldType {
    std::shared_ptr<FieldType> elementType;

    VectorFieldType(std::shared_ptr<FieldType> elementType);

    void init(void* ptr) const override;
    void free(void* ptr) const override;
    std::string asString() const override;
    size_t getSize() const override;
    size_t getAlignment() const override;
};

struct MapFieldType : public FieldType {
    std::shared_ptr<FieldType> keyType;
    std::shared_ptr<FieldType> valueType;

    MapFieldType(std::shared_ptr<FieldType> keyType, std::shared_ptr<FieldType> valueType);

    void init(void* ptr) const override;
    void free(void* ptr) const override;
    std::string asString() const override;
};

struct StructFieldType : public FieldType {
    std::string name;

    StructFieldType(const std::string& name);

    void init(void* ptr) const override;
    void free(void* ptr) const override;
    std::string asString() const override;
};

struct EnumType {
    std::shared_ptr<PrimitiveFieldType> underlyingType;
    std::vector<std::pair<std::string, int64_t>> values;

    EnumType(const std::vector<std::string>& valueNames);

    std::string asString() const;
};

struct StructType {
    using Field = std::pair<std::string, std::shared_ptr<FieldType>>;
    std::vector<Field> fields;

    StructType();

    std::string asString() const;
};

// These functions take a std::shared_ptr, because the functions that use them
// sometimes need to replace pointers

template <typename R = void, typename Func>
R visit(Func&& func, std::shared_ptr<FieldType>& fieldType)
{
    switch (fieldType->fieldType) {
    case FieldType::Error:
        return func(std::dynamic_pointer_cast<ErrorFieldType>(fieldType));
    case FieldType::Builtin:
        return func(std::dynamic_pointer_cast<PrimitiveFieldType>(fieldType));
    case FieldType::String:
        return func(std::dynamic_pointer_cast<StringFieldType>(fieldType));
    case FieldType::Enum:
        return func(std::dynamic_pointer_cast<EnumFieldType>(fieldType));
    case FieldType::Struct:
        return func(std::dynamic_pointer_cast<StructFieldType>(fieldType));
    case FieldType::Array:
        return func(std::dynamic_pointer_cast<ArrayFieldType>(fieldType));
    case FieldType::Vector:
        return func(std::dynamic_pointer_cast<VectorFieldType>(fieldType));
    case FieldType::Map:
        return func(std::dynamic_pointer_cast<MapFieldType>(fieldType));
    default:
        assert(false && "Invalid FieldType");
    }
}

template <typename Func>
void traverse(Func&& func, std::shared_ptr<FieldType>& fieldType)
{
    func(fieldType);
    if (fieldType->fieldType == FieldType::Array) {
        traverse(func, dynamic_cast<ArrayFieldType*>(fieldType.get())->elementType);
    } else if (fieldType->fieldType == FieldType::Vector) {
        traverse(func, dynamic_cast<VectorFieldType*>(fieldType.get())->elementType);
    } else if (fieldType->fieldType == FieldType::Map) {
        const auto mapFieldType = dynamic_cast<MapFieldType*>(fieldType.get());
        traverse(func, mapFieldType->keyType);
        traverse(func, mapFieldType->valueType);
    }
}

}
