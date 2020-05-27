#pragma once

#include <bitset>
#include <cstdint>
#include <queue>
#include <utility>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <boost/dynamic_bitset.hpp>

#include "id.hpp"
#include "struct.hpp"

namespace myl {

struct EntityIdTag {
};
using EntityId = Id<EntityIdTag, size_t>;

constexpr auto maxComponents = 64;
struct ComponentIdTag {
};
using ComponentId = Id<ComponentIdTag, size_t, maxComponents>;

class ComponentPool {
public:
    ComponentPool(size_t componentSize, size_t pageSize = 0);

    bool has(EntityId entityId) const;
    void* add(EntityId entityId);
    // No const overload, because you probably never have a const ComponentPool anyways
    void* get(EntityId entityId);
    void remove(EntityId entityId);

private:
    struct Page {
        std::unique_ptr<void, void (*)(void*)> data;
        boost::dynamic_bitset<> occupied;

        Page();
    };

    std::pair<size_t, size_t> getIndices(EntityId entityId) const;
    void* getPointer(size_t page, size_t index);

    size_t componentSize_;
    size_t pageSize_;
    std::vector<Page> pages_;
};

class Component {
public:
    Component(const std::string& name, Struct&& s);

    ComponentId getId() const;
    const std::string& getName() const;
    const Struct& getStruct() const;

private:
    ComponentId id_;
    std::string name_;
    Struct struct_;
};

class ComponentMask {
public:
    ComponentMask() = default;
    ComponentMask(ComponentId id);

    bool includes(ComponentId id) const;
    bool includes(const ComponentMask& other) const;

    // Returns true if none of the components in other are included in *this
    bool includesNot(const ComponentMask& other) const;

    void add(ComponentId id);
    void remove(ComponentId id);

    ComponentMask operator+(ComponentId id) const;
    ComponentMask operator+(const ComponentMask& other) const;

    ComponentMask& operator+=(ComponentId id);
    ComponentMask& operator-=(ComponentId id);

    void clear();

    auto getMask() const;

private:
    ComponentMask(const std::bitset<maxComponents>& mask);

    std::bitset<maxComponents> mask_;
};

ComponentMask operator+(ComponentId a, ComponentId b);

class World {
public:
    struct System {
        std::string name;
        std::function<void(float)> function;
        double lastDuration = 0.0;
        bool enabled = true;

        template <typename Func>
        System(const std::string& name, Func&& func)
            : name(name)
            , function(std::forward<Func>(func))
        {
        }
    };

    World() = default;
    ~World();

    bool entityExists(EntityId id) const;

    EntityId newEntity();

    void destroyEntity(EntityId id);

    // Implement foreachEntity in the future that returns a custom iterator.
    std::vector<EntityId> getEntities(const ComponentMask& mask = ComponentMask()) const;

    void registerComponent(const std::string& name, Struct&& strct);

    const Component& getComponent(ComponentId compId) const;

    const std::vector<Component>& getComponents();

    bool hasComponent(EntityId id, ComponentId compId);

    template <typename T = void>
    T* addComponent(EntityId id, ComponentId compId)
    {
        assert(!hasComponent(id, compId));
        entities_[static_cast<size_t>(id)].components += compId;
        return reinterpret_cast<T*>(componentPools_[static_cast<size_t>(compId)].add(id));
    }

    template <typename T = void>
    T* getComponent(EntityId id, ComponentId compId)
    {
        assert(hasComponent(id, compId));
        return reinterpret_cast<T*>(getComponentBuffer(id, compId));
    }

    void removeComponent(EntityId id, ComponentId compId);

    /*
     * Enabling/Disabling: There are two places where a (in a way) an entity-component
     * association is saved. Once here in the world (with a ComponentMask in Entity)
     * and once more indirectly in the ComponentPool.
     * Disabling essentially means removing the component from the mask, but keeping it
     * in the pool. It is component removal without throwing away the data.
     * Some implications:
     * - hasComponent is false for disabled components
     * - addComponent after setComponentDisabled just adds/enables it back without
     * modifying/overwriting.
     * - removeComponent after setComponentDisabled throws away the data too.
     */

    void setComponentEnabled(EntityId id, ComponentId compId, bool enabled = true);
    void setComponentDisabled(EntityId id, ComponentId compId);

    // This is mostly for internal use? I need it in the Entity Inspector
    bool isComponentAllocated(EntityId id, ComponentId compId);
    void* getComponentBuffer(EntityId id, ComponentId compId);

    ComponentId getComponentId(const std::string& name) const;

    template <typename Func>
    void registerSystem(const std::string& name, Func&& func)
    {
        // This is kind of a hack to sort the internal systems vector of World,
        // but for some things I want it to be sorted.
        auto it = systems_.begin();
        while (it != systems_.end() && it->name < name)
            it++;
        systems_.emplace(it, name, std::forward<Func>(func));

        systemNames_.clear();
        for (size_t i = 0; i < systems_.size(); ++i)
            systemNames_.emplace(systems_[i].name, i);
    }

    void invokeSystem(const std::string& name, float dt);

    std::vector<System>& getSystems();

    void setSystemEnabled(const std::string& name, bool enabled = true);
    void setSystemDisabled(const std::string& name);

private:
    struct Entity {
        bool exists;
        ComponentMask components;
    };

    std::vector<Component> components_;
    boost::container::flat_map<std::string, ComponentId> componentNames_;
    std::vector<ComponentPool> componentPools_;

    std::vector<Entity> entities_;
    std::priority_queue<EntityId, std::vector<EntityId>, IdGreater<EntityId>> entityIdFreeList_;

    std::vector<System> systems_;
    boost::container::flat_map<std::string, size_t> systemNames_;
};

World& getDefaultWorld();

bool entityExists(EntityId id);
EntityId newEntity();
void destroyEntity(EntityId id);
std::vector<EntityId> getEntities(const ComponentMask& mask = ComponentMask());

void registerComponent(const std::string& name, Struct&& strct);
const Component& getComponent(ComponentId compId);
const std::vector<Component>& getComponents();

bool hasComponent(EntityId id, ComponentId compId);

template <typename T = void>
T* addComponent(EntityId id, ComponentId compId)
{
    return getDefaultWorld().addComponent<T>(id, compId);
}

template <typename T = void>
T* getComponent(EntityId id, ComponentId compId)
{
    return getDefaultWorld().getComponent<T>(id, compId);
}

void removeComponent(EntityId id, ComponentId compId);

void setComponentEnabled(EntityId id, ComponentId compId, bool enabled = true);
void setComponentDisabled(EntityId id, ComponentId compId);

bool isComponentAllocated(EntityId id, ComponentId compId);
void* getComponentBuffer(EntityId id, ComponentId compId);

ComponentId getComponentId(const std::string& name);

template <typename Func>
void registerSystem(const std::string& name, Func&& func)
{
    getDefaultWorld().registerSystem(name, std::forward<Func>(func));
}

void invokeSystem(const std::string& name, float dt);

std::vector<World::System>& getSystems();

void setSystemEnabled(const std::string& name, bool enabled = true);
void setSystemDisabled(const std::string& name);

template <typename T>
class SystemData {
public:
    SystemData(ComponentId componentId)
        : SystemData(getDefaultWorld(), componentId)
    {
    }

    SystemData(World& world, ComponentId componentId)
        : world_(world)
        , boundComponent_(componentId)
        , data_(sizeof(T))
    {
    }

    ~SystemData()
    {
        for (auto entityId : world_.getEntities()) {
            if (has(entityId))
                remove(entityId);
        }
    }

    bool has(EntityId id) const
    {
        return data_.has(id);
    }

    template <typename... Args>
    T& add(EntityId id, Args&&... args)
    {
        T* p = reinterpret_cast<T*>(data_.add(id));
        return *new (p) T(std::forward<Args>(args)...);
    }

    // addDefault is a template parameter, because this would not compile for
    // non-default-constructible types T.
    template <bool addDefault = false>
    T& get(EntityId id)
    {
        if constexpr (addDefault) {
            if (!has(id))
                return add(id);
        }
        return *reinterpret_cast<T*>(data_.get(id));
    }

    void remove(EntityId id)
    {
        T* ptr = reinterpret_cast<T*>(data_.get(id));
        ptr->~T();
        data_.remove(id);
    }

    void remove()
    {
        for (auto entityId : world_.getEntities()) {
            if (has(entityId) && !world_.hasComponent(entityId, boundComponent_))
                remove(entityId);
        }
    }

private:
    World& world_;
    ComponentId boundComponent_;
    ComponentPool data_;
};

}