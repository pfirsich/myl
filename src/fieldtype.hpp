#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

struct FieldType {
    enum Type { invalid, error, builtin, enum_, struct_, array, vector, map } fieldType;

    FieldType(Type fieldType);
    virtual ~FieldType() = default;

    virtual std::string asString() const = 0;
};

struct ErrorFieldType : public FieldType {
    std::string typeName;

    ErrorFieldType(const std::string& typeName);

    std::string asString() const;
};

struct BuiltinFieldType : public FieldType {
    enum Type {
        invalid,
        bool_,
        u8,
        i8,
        u16,
        i16,
        u32,
        i32,
        u64,
        i64,
        f32,
        vec2,
        vec3,
        vec4,
        string
    } type;

    static const std::map<std::string, Type> typeFromString;

    BuiltinFieldType(Type type);

    std::string asString() const;
};

struct EnumFieldType : public FieldType {
    std::string name;

    EnumFieldType(const std::string& name);

    std::string asString() const;
};

struct ArrayFieldType : public FieldType {
    std::shared_ptr<FieldType> elementType;
    size_t size;

    ArrayFieldType(std::shared_ptr<FieldType> elementType, size_t size);

    std::string asString() const;
};

struct VectorFieldType : public FieldType {
    std::shared_ptr<FieldType> elementType;

    VectorFieldType(std::shared_ptr<FieldType> elementType);

    std::string asString() const;
};

struct MapFieldType : public FieldType {
    std::shared_ptr<FieldType> keyType;
    std::shared_ptr<FieldType> valueType;

    MapFieldType(std::shared_ptr<FieldType> keyType, std::shared_ptr<FieldType> valueType);

    std::string asString() const;
};

struct StructFieldType : public FieldType {
    std::string name;

    StructFieldType(const std::string& name);

    std::string asString() const;
};

struct EnumType {
    std::shared_ptr<BuiltinFieldType> underlyingType;
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