#pragma once

#include <cstdint>
#include <cstdlib>

#include "fieldtype.hpp"

namespace myl {

class Vector {
public:
    Vector(FieldType* elementType)
        : elementType_(elementType)
    {
    }

    ~Vector()
    {
        resize(0);
        std::free(data_);
    }

    void* getPointer(size_t index)
    {
        return data_ + index * elementType_->getSize();
    }

    template <typename T>
    T& get(size_t index)
    {
        assert(sizeof(T) == elementType_->getSize());
        assert(index < size_);
        return *reinterpret_cast<T*>(getPointer(index));
    }

    template <typename T>
    T& getBack(size_t offset = 0)
    {
        return get<T>(size_ - 1 - offset);
    }

    void pushBack(size_t num = 1)
    {
        resize(size_ + num);
    }

    void popBack(size_t num = 1)
    {
        assert(size_ >= num);
        resize(size_ - num);
    }

    void resize(size_t newSize)
    {
        if (newSize > size_) {
            const auto oldSize = size_;
            resize_(newSize);
            std::memset(getPointer(oldSize), 0, (newSize - oldSize) * elementType_->getSize());
            for (size_t i = oldSize; i < size_; ++i)
                elementType_->init(getPointer(i));
        } else {
            for (size_t i = newSize; i < size_; ++i)
                elementType_->free(getPointer(i));
            resize_(newSize);
        }
    }

    template <typename T = void>
    T* getData()
    {
        return data_;
    }

    size_t getSize() const
    {
        return size_;
    }

    size_t getCapacity() const
    {
        return capacity_;
    }

    size_t getElementSize() const
    {
        return elementType_->getSize();
    }

private:
    void resize_(size_t newSize)
    {
        if (capacity_ < newSize) {
            const auto oldData = data_;
            const auto newCap = std::max(size_ * 2, newSize);
            data_ = reinterpret_cast<uint8_t*>(std::malloc(newCap * elementType_->getSize()));
            capacity_ = newCap;
            std::memcpy(data_, oldData, size_ * elementType_->getSize());
            std::free(oldData);
        }
        size_ = newSize;
    }

    uint8_t* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
    // Yes, this should be a std::shared_ptr<FieldType>, but I want to share this struct
    // with e.g. LuaJIT FFI, so it's a pointer instead. I thought about using a
    // std::shared_ptr<FieldType>*, but I think this is fine.
    FieldType* elementType_ = nullptr;
};

}
