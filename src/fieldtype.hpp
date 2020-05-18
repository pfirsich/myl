#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

std::string indent(std::string str, const char* indent = "  ")
{
    boost::replace_all(str, "\n", std::string("\n") + indent);
    return str;
}

struct FieldType {
    enum Type { invalid, placeholder, builtin, enum_, struct_, array, vector, map } fieldType;

    FieldType(Type fieldType)
        : fieldType(fieldType)
    {
    }
    virtual ~FieldType() = default;

    virtual std::string asString() const = 0;

    virtual void replacePlaceholder(const std::string& typeStr, std::shared_ptr<FieldType> type)
    {
    }
};

// This is a placeholder for when e.g. a vector has an elementType, that has not been parsed yet.
// In a second pass this will be replaced.
struct PlaceHolderFieldType : public FieldType {
    std::string typeStr;

    PlaceHolderFieldType(std::string typeStr)
        : FieldType(FieldType::placeholder)
        , typeStr(typeStr)
    {
    }

    std::string asString() const
    {
        return "placeholder(" + typeStr + ")";
    }
};

void replacePlaceholder(std::shared_ptr<FieldType>& fieldType, const std::string& typeStr,
    std::shared_ptr<FieldType> type)
{
    if (fieldType->fieldType == FieldType::placeholder) {
        const auto placeholderFieldType = dynamic_cast<PlaceHolderFieldType*>(fieldType.get());
        assert(placeholderFieldType);
        if (placeholderFieldType->typeStr == typeStr)
            fieldType = type;
    } else {
        fieldType->replacePlaceholder(typeStr, std::move(type));
    }
}

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

    BuiltinFieldType(Type type)
        : FieldType(FieldType::builtin)
        , type(type)
    {
    }

    std::string asString() const
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
};

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

struct EnumFieldType : public FieldType {
    // TODO: underlying type, assigned values
    std::vector<std::string> values;

    EnumFieldType(std::vector<std::string> values)
        : FieldType(FieldType::enum_)
        , values(std::move(values))
    {
    }

    std::string asString() const
    {
        std::stringstream ss;
        ss << "enum {";
        for (size_t i = 0; i < values.size(); ++i) {
            ss << values[i];
            if (i < values.size() - 1)
                ss << ", ";
        }
        ss << "}";
        return ss.str();
    }
};

struct StructFieldType : public FieldType {
    using Field = std::pair<std::string, std::shared_ptr<FieldType>>;
    std::vector<Field> fields;

    StructFieldType()
        : FieldType(FieldType::struct_)
    {
    }

    std::string asString() const
    {
        std::stringstream ss;
        ss << "struct {\n";
        for (size_t i = 0; i < fields.size(); ++i) {
            ss << indent(fields[i].first + " : " + fields[i].second->asString());
            if (i < fields.size() - 1)
                ss << ",\n";
            else
                ss << "\n";
        }
        ss << "}";
        return ss.str();
    }

    void replacePlaceholder(const std::string& typeStr, std::shared_ptr<FieldType> type)
    {
        for (auto& field : fields)
            ::replacePlaceholder(field.second, typeStr, type);
    }
};

struct ArrayFieldType : public FieldType {
    std::shared_ptr<FieldType> elementType;
    size_t size;

    ArrayFieldType(std::shared_ptr<FieldType> elementType, size_t size)
        : FieldType(FieldType::array)
        , elementType(std::move(elementType))
        , size(size)
    {
    }

    std::string asString() const
    {
        return "array<" + elementType->asString() + ", " + std::to_string(size) + ">";
    }

    void replacePlaceholder(const std::string& typeStr, std::shared_ptr<FieldType> type)
    {
        ::replacePlaceholder(elementType, typeStr, type);
    }
};

struct VectorFieldType : public FieldType {
    std::shared_ptr<FieldType> elementType;

    VectorFieldType(std::shared_ptr<FieldType> elementType)
        : FieldType(FieldType::vector)
        , elementType(std::move(elementType))
    {
    }

    std::string asString() const
    {
        return "vector<" + elementType->asString() + ">";
    }

    void replacePlaceholder(const std::string& typeStr, std::shared_ptr<FieldType> type)
    {
        ::replacePlaceholder(elementType, typeStr, type);
    }
};

struct MapFieldType : public FieldType {
    std::shared_ptr<FieldType> keyType;
    std::shared_ptr<FieldType> valueType;

    MapFieldType(std::shared_ptr<FieldType> keyType, std::shared_ptr<FieldType> valueType)
        : FieldType(FieldType::array)
        , keyType(std::move(keyType))
        , valueType(std::move(valueType))
    {
    }

    std::string asString() const
    {
        return "map<" + keyType->asString() + ", " + valueType->asString() + ">";
    }

    void replacePlaceholder(const std::string& typeStr, std::shared_ptr<FieldType> type)
    {
        ::replacePlaceholder(keyType, typeStr, type);
        ::replacePlaceholder(valueType, typeStr, type);
    }
};