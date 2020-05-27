#pragma once

#include <cstdint>
#include <cstdlib>

namespace myl {

class Vector {
public:
    Vector(size_t elementSize)
        : elementSize_(elementSize)
    {
    }

    ~Vector()
    {
        std::free(data_);
    }

    void* getPointer(size_t index)
    {
        return data_ + index * elementSize_;
    }

    template <typename T>
    T& get(size_t index)
    {
        assert(sizeof(T) == elementSize_);
        assert(index < size_);
        return *reinterpret_cast<T*>(getPointer(index));
    }

    template <typename T>
    T& getBack(size_t offset = 0)
    {
        return get<T>(size_ - 1 - offset);
    }

    template <typename T>
    T& pushBack(const T& val)
    {
        assert(sizeof(T) == elementSize_);
        resize_(size_ + 1);
        std::memcpy(getPointer(size_ - 1), &val, sizeof(T));
    }

    void pushBack()
    {
        resize(size_ + 1);
    }

    void popBack()
    {
        resize_(size_ - 1);
    }

    void resize(size_t newSize)
    {
        const auto oldSize = size_;
        resize_(newSize);
        if (newSize > oldSize)
            std::memset(getPointer(oldSize), 0, (newSize - oldSize) * elementSize_);
    }

    template <typename T>
    void resize(size_t newSize, const T& val)
    {
        assert(sizeof(T) == elementSize_);
        const auto oldSize = size_;
        resize_(newSize);
        for (size_t i = oldSize; i < newSize; ++i)
            std::memcpy(getPointer(i), &val, sizeof(T));
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
        return elementSize_;
    }

private:
    void resize_(size_t newSize)
    {
        if (capacity_ < newSize) {
            const auto oldData = data_;
            const auto newCap = std::max(size_ * 2, newSize);
            data_ = reinterpret_cast<uint8_t*>(std::malloc(newCap * elementSize_));
            capacity_ = newCap;
            std::memcpy(data_, oldData, size_ * elementSize_);
            std::free(oldData);
        }
        size_ = newSize;
    }

    uint8_t* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
    size_t elementSize_ = 0;
};

}