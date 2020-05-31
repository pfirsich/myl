#pragma once

#include <glm/glm.hpp>

#include "color.hpp"
#include "fieldtype.hpp"
#include "structstring.hpp"
#include "structvector.hpp"

namespace myl {

/*
 * According to a conversation with someone more knowledgable:
 * Each platform bases it's structure layout (alignment, sizes)
 * on one compiler (gcc for Linux, * Clang for *BSD, MSVC for Windows).
 * So I think if I just use std::alignment_of with a compiler that already
 * plays well with the platform it's working on, then I should be good.
 */

class Struct {
public:
    friend class StructBuilder;

    struct Field {
        std::string name;
        std::shared_ptr<FieldType> type;
        size_t offset;
        size_t size;
        size_t alignment;
    };

    const std::vector<Field>& getFields() const;

    size_t getSize() const;

    size_t getAlignment() const;

    void init(void* ptr) const;
    void free(void* ptr) const;

private:
    Struct(const std::vector<Field>& fields, size_t size, size_t alignment);

    static void freeField(std::shared_ptr<FieldType> fieldType, uint8_t* ptr);
    static void initField(std::shared_ptr<FieldType> fieldType, uint8_t* ptr);

    std::vector<Field> fields_;
    size_t size_;
    size_t alignment_;
};

constexpr size_t padding(size_t offset, size_t alignment);

constexpr size_t align(size_t offset, size_t alignment);

class StructBuilder {
public:
    StructBuilder();

    void addField(const std::string& name, std::shared_ptr<FieldType> type);

    template <typename T>
    void addField(const std::string& /*name*/)
    {
        static_assert(std::is_same_v<std::decay_t<T>, T>, "Please only use value types");
        static_assert(std::is_same_v<T, void> && !std::is_same_v<T, void>,
            "No specialization for this type.");
    }

    template <typename T, typename M>
    StructBuilder addField(const std::string& name, M T::* /*field*/)
    {
        addField<M>(name);
        return *this;
    }

    Struct build() const;

private:
    std::vector<Struct::Field> fields_;
    size_t currentOffset_;
};

template <>
inline void StructBuilder::addField<glm::vec2>(const std::string& name)
{
    addField(name, std::make_shared<PrimitiveFieldType>(PrimitiveFieldType::vec2));
}

template <>
inline void StructBuilder::addField<glm::vec3>(const std::string& name)
{
    addField(name, std::make_shared<PrimitiveFieldType>(PrimitiveFieldType::vec3));
}

template <>
inline void StructBuilder::addField<glm::vec4>(const std::string& name)
{
    addField(name, std::make_shared<PrimitiveFieldType>(PrimitiveFieldType::vec4));
}

template <>
inline void StructBuilder::addField<Color>(const std::string& name)
{
    addField(name, std::make_shared<PrimitiveFieldType>(PrimitiveFieldType::color));
}

template <>
inline void StructBuilder::addField<float>(const std::string& name)
{
    addField(name, std::make_shared<PrimitiveFieldType>(PrimitiveFieldType::f32));
}

}
