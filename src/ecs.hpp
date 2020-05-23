#pragma once

#include <bitset>
#include <cstdint>
#include <map>
#include <queue>
#include <utility>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/dynamic_bitset.hpp>

#include "struct.hpp"

using EntityId = uint32_t;
constexpr auto maxComponents = 64;

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

        if (page >= pages_.size())
            pages_.resize(page + 1, Page(pageSize_));

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
        boost::dynamic_bitset<> occupied;

        Page(size_t pageSize)
            : occupied(pageSize)
        {
        }
    };

    std::pair<size_t, size_t> getIndices(EntityId entityId) const
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

class Component {
public:
    using Id = size_t;

    Component(const std::string& name, Struct&& s)
        : id_(getComponentId())
        , name_(name)
        , struct_(s)
    {
    }

    Id getId() const
    {
        return id_;
    }

    const std::string& getName() const
    {
        return name_;
    }

    const Struct& getStruct() const
    {
        return struct_;
    }

private:
    static Id getComponentId()
    {
        static Id counter = 0;
        assert(counter < maxComponents);
        return counter++;
    }

    Id id_;
    std::string name_;
    Struct struct_;
};

class ComponentMask {
public:
    ComponentMask() = default;

    ComponentMask(const Component& component)
    {
        mask_.set(component.getId(), true);
    }

    bool includes(Component::Id id) const
    {
        return mask_.test(id);
    }

    bool includes(const ComponentMask& other) const
    {
        return (mask_ & other.mask_) == other.mask_;
    }

    bool includes(const Component& component) const
    {
        return mask_.test(component.getId());
    }

    // Returns true if none of the components in other are included in *this
    bool includesNot(const ComponentMask& other) const
    {
        return (mask_ & other.mask_).none();
    }

    ComponentMask operator+(const ComponentMask& other) const
    {
        return ComponentMask(mask_ | other.mask_);
    }

    ComponentMask operator+(const Component& component) const
    {
        ComponentMask mask(*this);
        mask.mask_.set(component.getId(), true);
        return mask;
    }

    void clear()
    {
        mask_.reset();
    }

private:
    ComponentMask(const std::bitset<maxComponents>& mask)
        : mask_(mask)
    {
    }

    std::bitset<maxComponents> mask_;
};

class World {
public:
    World(const std::vector<Component>& components)
    {
        componentPools_.reserve(components.size());
        componentNames_.reserve(components.size());
        for (const auto& component : components) {
            // TODO: Page size has to be configurable at some point.
            componentPools_.emplace_back(component.getStruct().getSize(), 64);
            componentNames_.emplace(component.getName(), component.getId());
        }
    }

    bool entityExists(EntityId id) const
    {
        return id < entities_.size() && entities_[id].exists;
    }

    EntityId newEntity()
    {
        if (entityIdFreeList_.empty()) {
            entities_.push_back(Entity { true, ComponentMask() });
            return entities_.size() - 1;
        } else {
            const auto id = entityIdFreeList_.top();
            entityIdFreeList_.pop();
            assert(id < entities_.size());
            entities_[id].exists = true;
            entities_[id].components.clear();
            return id;
        }
    }

    void destroyEntity(EntityId id)
    {
        assert(entityExists(id));
        for (size_t compId = 0; compId < componentPools_.size(); ++compId) {
            if (entities_[id].components.includes(compId)) {
                componentPools_[compId].remove(id);
            }
        }
        entities_[id].exists = false;
        entityIdFreeList_.push(id);
    }

    template <typename T = void>
    T* addComponent(EntityId id, Component::Id compId)
    {
        return reinterpret_cast<T*>(componentPools_[compId].add(id));
    }

    template <typename T = void>
    T* addComponent(EntityId id, const std::string& name)
    {
        return addComponent<T>(id, componentNames_.at(name));
    }

    template <typename T = void>
    T* getComponent(EntityId id, Component::Id compId)
    {
        return reinterpret_cast<T*>(componentPools_[compId].get(id));
    }

    template <typename T = void>
    T* getComponent(EntityId id, const std::string& name)
    {
        return getComponent<T>(id, componentNames_.at(name));
    }

    void removeComponent(EntityId id, Component::Id compId)
    {
        componentPools_[compId].remove(id);
    }

    void removeComponent(EntityId id, const std::string& name)
    {
        removeComponent(id, componentNames_.at(name));
    }

    Component::Id getComponentId(const std::string& name) const
    {
        return componentNames_.at(name);
    }

private:
    struct Entity {
        bool exists;
        ComponentMask components;
    };

    std::vector<ComponentPool> componentPools_;
    boost::container::flat_map<std::string, Component::Id> componentNames_;
    std::vector<Entity> entities_;
    std::priority_queue<EntityId, std::vector<EntityId>, std::greater<>> entityIdFreeList_;
};