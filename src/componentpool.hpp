#pragma once

#include <utility>
#include <vector>

#include <boost/dynamic_bitset.hpp>

using EntityId = uint32_t;

class ComponentPool {
public:
    ComponentPool(size_t componentSize, size_t pageSize)
        : componentSize_(componentSize)
        , pageSize_(pageSize)
    {
    }

    bool has(EntityId entityId) const
    {
        const auto [page, index] = getIndices(entityId);
        return pages_.size() > page && pages_[page].occupied[index];
    }

    void* add(EntityId entityId)
    {
        assert(!has(entityId));
        const auto [page, index] = getIndices(entityId);

        if (page >= pages_[page].size())
            pages_.resize(page + 1);

        auto& pageObj = pages_[page];
        if (!pageObj.data)
            pageObj.data = ::operator new(pageSize_* componentSize_);
        pageObj.occupied.set(index, true);

        return getPointer(page, index);
    }

    // No const overload, because you probably never have a const ComponentPool anyways
    void* get(EntityId entityId)
    {
        assert(has(entityId));
        const auto [page, index] = getIndices(entityId);
        return getPointer(page, index);
    }

    void remove(EntityId entityId)
    {
        assert(has(entityId));
        const auto [page, index] = getIndices(entityId);
        auto& pageObj = pages_[page];
        pageObj.occupied.set(index, false);

        if (pageObj.occupied.none()) {
            ::operator delete(pageObj.data);
            pageObj.data = nullptr;
        }
    }

private:
    struct Page {
        void* data = nullptr;
        boost::dynamic_bitset occupied;
    };

    auto getIndices(EntityId entityId) const
    {
        return std::pair<size_t, size_t>(entityId / pageSize_, entityId % pageSize_);
    }

    void* getPointer(size_t page, size_t index)
    {
        assert(pages_[page].data);
        return reinterpret_cast<uint8_t*>(pages_[page].data) + componentSize_ * index;
    }

    size_t componentSize_;
    size_t pageSize_;
    std::vector<Page> pages_;
};