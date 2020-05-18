#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

// TODO: Paraphrase /home/joel/Pictures/alignment_rules.png

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
        template <typename T>
        T& get(void* structPtr) const
        {
            return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(structPtr) + offset);
        }

        template <typename T>
        const T& get(const void* structPtr) const
        {
            return *reinterpret_cast<const T*>(
                reinterpret_cast<const uint8_t*>(structPtr) + offset);
        }

        std::string name;
        size_t offset;
        size_t size;
        size_t alignment;
    };

    const std::vector<Field>& getFields() const
    {
        return fields_;
    }

    bool hasField(const std::string& name) const
    {
        return findField(name) == fields_.end();
    }

    const Field& getField(const std::string& name) const
    {
        const auto it = findField(name);
        assert(it != fields_.end());
        return *it;
    }

    size_t getSize() const
    {
        return size_;
    }

    size_t getAlignment() const
    {
        return alignment_;
    }

    std::unique_ptr<uint8_t[]> allocate(size_t num = 1) const
    {
        assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= alignment_);
        return std::make_unique<uint8_t[]>(size_ * num);
    }

private:
    Struct(std::vector<Field> fields, size_t size, size_t alignment)
        : fields_(std::move(fields))
        , size_(size)
        , alignment_(alignment)
    {
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

    void addField(const std::string& name, size_t size, size_t alignment)
    {
        currentOffset_ = align(currentOffset_, alignment);
        fields_.emplace_back(Struct::Field { name, currentOffset_, size, alignment });
    }

    template <typename T>
    void addField(const std::string& name)
    {
        addField(name, sizeof(T), std::alignment_of_v<T>);
    }

    void addField(const Struct& s, const std::string& name)
    {
        addField(name, s.getSize(), s.getAlignment());
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