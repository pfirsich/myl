#include "structstring.hpp"

#include <cassert>

namespace myl {
String::String()
{
    assign(nullptr, 0); // to preallocate minAllocSize
}

String::String(const char* buf, size_t size)
{
    assign(buf, size);
}

String::String(const char* str)
    : String(str, std::strlen(str))
{
}

String::String(const std::string& str)
    : String(str.data(), str.size())
{
}

String::~String()
{
    std::free(data_);
}

void String::assign(const char* buf, size_t size)
{
    if (capacity_ < size + 1) {
        std::free(data_);
        const auto allocSize = std::max(minAllocSize, std::max(capacity_ * 2, size + 1));
        data_ = reinterpret_cast<char*>(std::malloc(allocSize));
        capacity_ = allocSize;
    }
    assert(data_);
    if (buf)
        std::memcpy(data_, buf, size);
    data_[size] = '\0';
    size_ = size;
}

void String::assign(const String& str)
{
    assign(str.getData(), str.getSize());
}

void String::assign(const char* str)
{
    assign(str, std::strlen(str));
}

void String::assign(const std::string& str)
{
    assign(str.data(), str.size());
}

String& String::operator=(const String& str)
{
    assign(str);
    return *this;
}

String& String::operator=(const char* str)
{
    assign(str);
    return *this;
}

String& String::operator=(const std::string& str)
{
    assign(str);
    return *this;
}

std::string String::str() const
{
    return std::string(data_, size_);
}

String::operator std::string() const
{
    return str();
}

const char* String::getData() const
{
    return data_;
}

size_t String::getSize() const
{
    return size_;
}

size_t String::getCapacity() const
{
    return capacity_;
}
}
