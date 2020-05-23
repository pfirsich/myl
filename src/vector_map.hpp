#pragma once

#include <algorithm>
#include <memory>
#include <vector>

// like flat_map, but keeps insertion order and does linear search
// Tries to be STL compatible
template <typename Key, typename T, typename Allocator = std::allocator<std::pair<const Key, T>>>
class vector_map {
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const key_type, mapped_type>;
    using char_type = typename key_type::value_type;
    using size_type = std::size_t;
    using allocator_type = Allocator;
    using reference = mapped_type&;
    using const_reference = const mapped_type&;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

    using vec_type = std::vector<value_type>;

    using iterator = typename vec_type::iterator;
    using const_iterator = typename vec_type::const_iterator;
    using reverse_iterator = typename vec_type::reverse_iterator;
    using const_reverse_iterator = typename vec_type::const_reverse_iterator;

    vector_map() = default;
    ~vector_map() = default;

    /* Selectors */
    const_iterator find(const key_type& key) const
    {
        return std::find_if(
            data_.cbegin(), data_.cend(), [&](const value_type& v) { return v.first == key; });
    }

    iterator find(const key_type& key)
    {
        return std::find_if(
            data_.begin(), data_.end(), [&](const value_type& v) { return v.first == key; });
    }

    size_type size() const
    {
        return data_.size();
    }

    bool empty() const
    {
        return data_.empty();
    }

    reference at(const key_type& key)
    {
        const auto it = find(key);
        if (it == data_.end())
            throw std::out_of_range("Could not find key in vector_map");
        return *it;
    }

    const_reference at(const key_type& key) const
    {
        const auto it = find(key);
        if (it == data_.end())
            throw std::out_of_range("Could not find key in vector_map");
        return *it;
    }

    /* Mutators */
    std::pair<iterator, bool> insert(value_type&& value)
    {
        const auto it = find(value.first);
        if (it != data_.end())
            return std::make_pair(it, false);

        data_.emplace_back(value);
        return std::make_pair(std::prev(data_.end()), true);
    }

    std::pair<iterator, bool> insert(const key_type& key, const mapped_type& value)
    {
        return insert(value_type(key, value));
    }

    reference operator[](const key_type& key)
    {
        const auto it = find(key);
        if (it == data_.end()) {
            data_.emplace_back(key, mapped_type());
            return data_.back();
        }
        return it->second;
    }

    void erase(const key_type& key)
    {
        const auto it = find(key);
        if (it == data_.end())
            return;
        data_.erase(it);
    }

    iterator erase(iterator pos)
    {
        data_.erase(pos);
    }

    void clear()
    {
        data_.clear();
    }

    /* Iterators */
    iterator begin()
    {
        return data_.begin();
    }

    const_iterator begin() const
    {
        return data_.begin();
    }

    const_iterator cbegin() const
    {
        return data_.cbegin();
    }

    iterator end()
    {
        return data_.end();
    }

    const_iterator end() const
    {
        return data_.end();
    }

    const_iterator cend() const
    {
        return data_.cend();
    }

    reverse_iterator rbegin()
    {
        return data_.rbegin();
    }

    const_reverse_iterator rbegin() const
    {
        return data_.rbegin();
    }

    const_reverse_iterator crbegin() const
    {
        return data_.crbegin();
    }

    reverse_iterator rend()
    {
        return data_.rend();
    }

    const_reverse_iterator rend() const
    {
        return data_.rend();
    }

    const_reverse_iterator crend() const
    {
        return data_.crend();
    }

private:
    vec_type data_;
};