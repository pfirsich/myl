#pragma once

#include <cstdlib>
#include <cstring>
#include <string>

namespace myl {

class String {
    static constexpr size_t minAllocSize = 32;

public:
    String();
    String(const char* buf, size_t size);
    String(const char* str);
    String(const std::string& str);
    ~String();

    void assign(const char* buf, size_t size);
    void assign(const String& str);
    void assign(const char* str);
    void assign(const std::string& str);

    String& operator=(const String& str);
    String& operator=(const char* str);
    String& operator=(const std::string& str);

    std::string str() const;
    operator std::string() const;

    const char* getData() const;
    size_t getSize() const;
    size_t getCapacity() const;

private:
    char* data_ = nullptr;
    size_t size_ = 0; // in characters
    size_t capacity_ = 0; // (counts '\0' - the size of the buffer pointed to by data)
};

}
