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
    // I can theoretically do this once in an empty buffer and then just copy it over each time
    for (const auto& field : fields_)
        field.type->init(reinterpret_cast<uint8_t*>(ptr) + field.offset);
}

void Struct::free(void* ptr) const
{
    for (const auto& field : fields_)
        field.type->free(reinterpret_cast<uint8_t*>(ptr) + field.offset);
}

Struct::Struct(const std::vector<Field>& fields, size_t size, size_t alignment)
    : fields_(std::move(fields))
    , size_(size)
    , alignment_(alignment)
{
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
