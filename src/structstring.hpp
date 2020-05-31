#pragma once

#include <cstdlib>
#include <cstring>
#include <string>

namespace myl {

class String {
    static constexpr size_t minAllocSize = 32;

public:
    String()
    {
        assign(nullptr, 0);
    }

    String(const char* buf, size_t size)
    {
        assign(buf, size);
    }

    String(const char* str)
        : String(str, std::strlen(str))
    {
    }

    String(const std::string& str)
        : String(str.data(), str.size())
    {
    }

    ~String()
    {
        std::free(data_);
    }

    void assign(const char* buf, size_t size)
    {
        if (capacity_ < size + 1) {
            std::free(data_);
            const auto allocSize = std::max(minAllocSize, std::max(capacity_ * 2, size + 1));
            data_ = reinterpret_cast<char*>(std::malloc(allocSize));
            capacity_ = allocSize;
        }
        std::memcpy(data_, buf, size);
        data_[size] = '\0';
        size_ = size;
    }

    void assign(const String& str)
    {
        assign(str.getData(), str.getSize());
    }

    void assign(const char* str)
    {
        assign(str, std::strlen(str));
    }

    void assign(const std::string& str)
    {
        assign(str.data(), str.size());
    }

    String& operator=(const String& str)
    {
        assign(str);
        return *this;
    }

    String& operator=(const char* str)
    {
        assign(str);
        return *this;
    }

    String& operator=(const std::string& str)
    {
        assign(str);
        return *this;
    }

    std::string str() const
    {
        return std::string(data_, size_);
    }

    operator std::string() const
    {
        return str();
    }

    const char* getData() const
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

private:
    char* data_ = nullptr;
    size_t size_ = 0; // in characters
    size_t capacity_ = 0; // (counts '\0' - the size of the buffer pointed to by data)
};

}
