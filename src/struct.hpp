#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "fieldtype.hpp"

/*
 * According to a conversation with someone more knowledgable:
 * Each platform bases it's structure layout (alignment, sizes)
 * on one compiler (gcc for Linux, * Clang for *BSD, MSVC for Windows).
 * So I think if I just use std::alignment_of with a compiler that already
 * plays well with the platform it's working on, then I should be good.
 */

class Struct {
private:
    auto findField(const std::string& name) const
    {
        return std::find_if(fields_.begin(), fields_.end(),
            [&name](const Field& field) { return field.name == name; });
    }

public:
    friend class StructBuilder;

    struct Field {
        std::string name;
        std::shared_ptr<FieldType> type;
        size_t offset;
        size_t size;
        size_t alignment;
    };

    const std::vector<Field>& getFields() const
    {
        return fields_;
    }

    size_t getSize() const
    {
        return size_;
    }

    size_t getAlignment() const
    {
        return alignment_;
    }

    void free(void* ptr) const
    {
        for (const auto& field : fields_)
            freeField(field.type, reinterpret_cast<uint8_t*>(ptr) + field.offset);
    }

private:
    Struct(const std::vector<Field>& fields, size_t size, size_t alignment)
        : fields_(std::move(fields))
        , size_(size)
        , alignment_(alignment)
    {
    }

    static void freeField(const std::shared_ptr<FieldType>& fieldType, uint8_t* ptr)
    {
        if (fieldType->fieldType == FieldType::builtin) {
            const auto ft = std::dynamic_pointer_cast<BuiltinFieldType>(fieldType);
            if (ft->type == BuiltinFieldType::string)
                reinterpret_cast<String*>(ptr)->~String();
        } else if (fieldType->fieldType == FieldType::array) {
            const auto ft = std::dynamic_pointer_cast<ArrayFieldType>(fieldType);
            for (size_t i = 0; i < ft->size; ++i)
                freeField(ft->elementType, ptr + i * ft->elementType->getSize());
        } else if (fieldType->fieldType == FieldType::vector) {
            assert(false && "free vector field unimplemented");
        } else if (fieldType->fieldType == FieldType::map) {
            assert(false && "free map field unimplemented");
        } else if (fieldType->fieldType == FieldType::struct_) {
            assert(false && "free struct field unimplemented");
        }
    }

    std::vector<Field> fields_;
    size_t size_;
    size_t alignment_;
};

constexpr size_t padding(size_t offset, size_t alignment)
{
    const auto misalignment = offset % alignment;
    return misalignment > 0 ? alignment - misalignment : 0;
}

constexpr size_t align(size_t offset, size_t alignment)
{
    return offset + padding(offset, alignment);
}

class StructBuilder {
public:
    StructBuilder()
        : currentOffset_(0)
    {
    }

    void addField(const std::string& name, std::shared_ptr<FieldType> type)
    {
        const auto size = type->getSize();
        const auto alignment = type->getAlignment();
        currentOffset_ = align(currentOffset_, alignment);
        fields_.emplace_back(Struct::Field { name, type, currentOffset_, size, alignment });
        currentOffset_ += size;
    }

    template <typename T>
    void addField(const std::string& name)
    {
        static_assert(std::is_same_v<std::decay_t<T>, T>, "Please only use value types");
        static_assert(std::is_same_v<T, void> && !std::is_same_v<T, void>,
            "No specialization for this type.");
    }

    template <>
    void addField<glm::vec2>(const std::string& name)
    {
        addField(name, std::make_shared<BuiltinFieldType>(BuiltinFieldType::vec2));
    }

    template <>
    void addField<float>(const std::string& name)
    {
        addField(name, std::make_shared<BuiltinFieldType>(BuiltinFieldType::f32));
    }

    template <typename T, typename M>
    StructBuilder addField(const std::string& name, M T::*field)
    {
        addField<M>(name);
        return *this;
    }

    Struct build() const
    {
        const auto alignment = std::max_element(
            fields_.begin(), fields_.end(), [](const auto& lhs, const auto& rhs) {
                return lhs.alignment < rhs.alignment;
            })->alignment;
        const auto size = align(currentOffset_, alignment);

        return Struct { fields_, size, alignment };
    }

private:
    std::vector<Struct::Field> fields_;
    size_t currentOffset_;
};