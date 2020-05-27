#include "struct.hpp"

#include <algorithm>
#include <cassert>

namespace myl {

const std::vector<Struct::Field>& Struct::getFields() const
{
    return fields_;
}

size_t Struct::getSize() const
{
    return size_;
}

size_t Struct::getAlignment() const
{
    return alignment_;
}

void Struct::init(void* ptr) const
{
    for (const auto& field : fields_)
        initField(field.type, reinterpret_cast<uint8_t*>(ptr) + field.offset);
}

void Struct::free(void* ptr) const
{
    for (const auto& field : fields_)
        freeField(field.type, reinterpret_cast<uint8_t*>(ptr) + field.offset);
}

Struct::Struct(const std::vector<Field>& fields, size_t size, size_t alignment)
    : fields_(std::move(fields))
    , size_(size)
    , alignment_(alignment)
{
}

void Struct::initField(std::shared_ptr<FieldType> fieldType, uint8_t* ptr)
{
    // TODO: I only need to do this once into some buffer and then memcpy that to new structs
    // instead of doing it every time
    if (fieldType->fieldType == FieldType::vector) {
        const auto ft = std::dynamic_pointer_cast<VectorFieldType>(fieldType);
        // Can't next vectors yet, because I don't know what constructor to call when you push back
        // into them (elementSize)
        assert(ft->elementType->fieldType != FieldType::vector);
        new (ptr) Vector(ft->elementType->getSize());
    }
}

void Struct::freeField(std::shared_ptr<FieldType> fieldType, uint8_t* ptr)
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
        const auto ft = std::dynamic_pointer_cast<VectorFieldType>(fieldType);
        auto vecPtr = reinterpret_cast<Vector*>(ptr);
        for (size_t i = 0; vecPtr->getSize(); ++i)
            freeField(ft->elementType, reinterpret_cast<uint8_t*>(vecPtr->getPointer(i)));
        vecPtr->~Vector();
    } else if (fieldType->fieldType == FieldType::map) {
        assert(false && "free map field unimplemented");
    } else if (fieldType->fieldType == FieldType::struct_) {
        assert(false && "free struct field unimplemented");
    }
}

constexpr size_t padding(size_t offset, size_t alignment)
{
    const auto misalignment = offset % alignment;
    return misalignment > 0 ? alignment - misalignment : 0;
}

constexpr size_t align(size_t offset, size_t alignment)
{
    return offset + padding(offset, alignment);
}

StructBuilder::StructBuilder()
    : currentOffset_(0)
{
}

void StructBuilder::addField(const std::string& name, std::shared_ptr<FieldType> type)
{
    const auto size = type->getSize();
    const auto alignment = type->getAlignment();
    currentOffset_ = align(currentOffset_, alignment);
    fields_.emplace_back(Struct::Field { name, type, currentOffset_, size, alignment });
    currentOffset_ += size;
}

Struct StructBuilder::build() const
{
    const auto alignment
        = std::max_element(fields_.begin(), fields_.end(), [](const auto& lhs, const auto& rhs) {
              return lhs.alignment < rhs.alignment;
          })->alignment;
    const auto size = align(currentOffset_, alignment);

    return Struct { fields_, size, alignment };
}

}