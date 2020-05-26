#include <cassert>
#include <vector>

template <typename T>
class RingBuffer {
public:
    RingBuffer(size_t maxSize)
    {
        assert(maxSize > 0);
        data_.resize(maxSize);
    }

    void clear()
    {
        size_ = 0;
        nextIndex_ = 0;
    }

    void setMaxSize(size_t maxSize)
    {
        assert(maxSize > 0);
        // Figuring out what nextIndex_ should be here and potentially copying slices
        // around is too annoying, so I just clear!
        data_.resize(maxSize);
        clear();
    }

    void push(const T& val)
    {
        data_[nextIndex_] = val;
        nextIndex_ = (nextIndex_ + 1) % data_.size();
        if (size_ < data_.size())
            size_++;
    }

    size_t getIndex(size_t index) const
    {
        // If the buffer hasn't been filled up, the oldest value is at 0
        if (size_ < data_.size())
            return index % data_.size();
        // If we already wrapped around, the oldest value is at nextIndex_, so start counting there
        return (nextIndex_ + index) % data_.size();
    }

    const T& operator[](size_t index) const
    {
        return data_[getIndex(index)];
    }

    const T* getData() const
    {
        return reinterpret_cast<const T*>(data_.data());
    }

    size_t getSize() const
    {
        return size_;
    }

public:
    std::vector<T> data_;
    size_t nextIndex_ = 0;
    size_t size_ = 0;
};