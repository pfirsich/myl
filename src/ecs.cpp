#include "ecs.hpp"

#include <iostream>

#include "util.hpp"

namespace myl {

ComponentPool::ComponentPool(size_t componentSize, size_t pageSize)
    : componentSize_(componentSize)
    , pageSize_(pageSize)
{
    if (pageSize_ == 0)
        pageSize_ = 1024 / componentSize; // ~1KB per page
}

bool ComponentPool::has(EntityId entityId) const
{
    const auto [page, index] = getIndices(entityId);
    return pages_.size() > page && pages_[page].occupied.test(index);
}

void* ComponentPool::add(EntityId entityId)
{
    assert(!has(entityId));
    const auto [page, index] = getIndices(entityId);

    if (page >= pages_.size()) {
        const auto oldSize = pages_.size();
        pages_.resize(page + 1);
        for (size_t i = oldSize; i < pages_.size(); ++i)
            pages_[i].occupied.resize(pageSize_);
    }

    auto& pageObj = pages_[page];
    if (!pageObj.data)
        pageObj.data.reset(::operator new(pageSize_* componentSize_));
    pageObj.occupied.set(index, true);

    auto ptr = getPointer(page, index);
    std::memset(ptr, 0, componentSize_);
    return ptr;
}

// No const overload, because you probably never have a const ComponentPool anyways
void* ComponentPool::get(EntityId entityId)
{
    assert(has(entityId));
    const auto [page, index] = getIndices(entityId);
    return getPointer(page, index);
}

void ComponentPool::remove(EntityId entityId)
{
    assert(has(entityId));
    const auto [page, index] = getIndices(entityId);
    auto& pageObj = pages_[page];
    pageObj.occupied.set(index, false);

    if (pageObj.occupied.none())
        pageObj.data.reset();
}

ComponentPool::Page::Page()
    : data(nullptr, [](void* p) { ::operator delete(p); })
    , occupied()
{
}

std::pair<size_t, size_t> ComponentPool::getIndices(EntityId entityId) const
{
    const auto id = static_cast<size_t>(entityId);
    return std::pair<size_t, size_t>(id / pageSize_, id % pageSize_);
}

void* ComponentPool::getPointer(size_t page, size_t index)
{
    assert(pages_[page].data);
    return reinterpret_cast<uint8_t*>(pages_[page].data.get()) + componentSize_ * index;
}

Component::Component(const std::string& name, Struct&& s)
    : id_(ComponentId::getNew())
    , name_(name)
    , struct_(s)
{
}

ComponentId Component::getId() const
{
    return id_;
}

const std::string& Component::getName() const
{
    return name_;
}

const Struct& Component::getStruct() const
{
    return struct_;
}

ComponentMask::ComponentMask(ComponentId id)
{
    mask_.set(static_cast<size_t>(id), true);
}

bool ComponentMask::includes(ComponentId id) const
{
    return mask_.test(static_cast<size_t>(id));
}

bool ComponentMask::includes(const ComponentMask& other) const
{
    return (mask_ & other.mask_) == other.mask_;
}

// Returns true if none of the components in other are included in *this
bool ComponentMask::includesNot(const ComponentMask& other) const
{
    return (mask_ & other.mask_).none();
}

void ComponentMask::add(ComponentId id)
{
    mask_.set(static_cast<size_t>(id), true);
}

void ComponentMask::remove(ComponentId id)
{
    mask_.set(static_cast<size_t>(id), false);
}

ComponentMask ComponentMask::operator+(ComponentId id) const
{
    ComponentMask mask(*this);
    mask.mask_.set(static_cast<size_t>(id), true);
    return mask;
}

ComponentMask ComponentMask::operator+(const ComponentMask& other) const
{
    return ComponentMask(mask_ | other.mask_);
}

ComponentMask& ComponentMask::operator+=(ComponentId id)
{
    add(id);
    return *this;
}

ComponentMask& ComponentMask::operator-=(ComponentId id)
{
    remove(id);
    return *this;
}

void ComponentMask::clear()
{
    mask_.reset();
}

auto ComponentMask::getMask() const
{
    return mask_;
}

ComponentMask::ComponentMask(const std::bitset<maxComponents>& mask)
    : mask_(mask)
{
}

ComponentMask operator+(ComponentId a, ComponentId b)
{
    ComponentMask mask;
    mask.add(a);
    mask.add(b);
    return mask;
}

World::~World()
{
    // We just have to make sure we call Struct::free here, because the componentPool
    // can't do that itself. The actual freeing is however still done by the pool.
    for (size_t compId = 0; compId < components_.size(); ++compId) {
        auto& pool = componentPools_[compId];
        auto& strct = components_[compId].getStruct();
        for (size_t id = 0; id < entities_.size(); ++id) {
            if (pool.has(EntityId(id)))
                strct.free(pool.get(EntityId(id)));
        }
    }
}

const std::vector<Component>& World::getComponents()
{
    return components_;
}

bool World::entityExists(EntityId id) const
{
    const auto idx = static_cast<size_t>(id);
    return idx < entities_.size() && entities_[idx].exists;
}

EntityId World::newEntity()
{
    if (entityIdFreeList_.empty()) {
        entities_.push_back(Entity { true, ComponentMask() });
        return EntityId(entities_.size() - 1);
    } else {
        const auto id = entityIdFreeList_.top();
        entityIdFreeList_.pop();
        const auto idx = static_cast<size_t>(id);
        assert(idx < entities_.size());
        entities_[idx].exists = true;
        entities_[idx].components.clear();
        return id;
    }
}

void World::destroyEntity(EntityId id)
{
    const auto idx = static_cast<size_t>(id);
    assert(entityExists(id));
    for (size_t compId = 0; compId < componentPools_.size(); ++compId) {
        if (entities_[idx].components.includes(ComponentId(compId))) {
            componentPools_[compId].remove(id);
        }
    }
    entities_[idx].exists = false;
    entityIdFreeList_.push(id);
}

bool World::hasComponent(EntityId id, ComponentId compId)
{
    assert(entityExists(id));
    return entities_[static_cast<size_t>(id)].components.includes(compId);
}

void World::removeComponent(EntityId id, ComponentId compId)
{
    // This does not assert hasComponent, because we might remove a disabled component
    // If there isn't even a disabled component the ComponentPool::get will abort
    const auto compIndex = static_cast<size_t>(compId);
    auto ptr = componentPools_[compIndex].get(id);
    components_[compIndex].getStruct().free(ptr);
    entities_[static_cast<size_t>(id)].components -= compId;
    componentPools_[compIndex].remove(id);
}

void World::setComponentEnabled(EntityId id, ComponentId compId, bool enabled)
{
    if (enabled)
        entities_[static_cast<size_t>(id)].components += compId;
    else
        entities_[static_cast<size_t>(id)].components -= compId;
}

void World::setComponentDisabled(EntityId id, ComponentId compId)
{
    setComponentEnabled(id, compId, false);
}

bool World::isComponentAllocated(EntityId id, ComponentId compId)
{
    return componentPools_[static_cast<size_t>(compId)].has(id);
}

void* World::getComponentBuffer(EntityId id, ComponentId compId)
{
    return componentPools_[static_cast<size_t>(compId)].get(id);
}

ComponentId World::getComponentId(const std::string& name) const
{
    return componentNames_.at(name);
}

// Implement foreachEntity in the future that returns a custom iterator.
std::vector<EntityId> World::getEntities(const ComponentMask& mask) const
{
    std::vector<EntityId> ids;
    for (size_t id = 0; id < entities_.size(); ++id) {
        auto& entity = entities_[id];
        if (entity.exists && entity.components.includes(mask))
            ids.push_back(EntityId(id));
    }
    return ids;
}

void World::registerComponent(const std::string& name, Struct&& strct)
{
    // Component names must be unique
    assert(std::all_of(components_.begin(), components_.end(),
        [&name](const Component& c) { return name != c.getName(); }));
    components_.emplace_back(name, std::forward<Struct>(strct));

    const Component& component = components_.back();
    // This whole design is horrible, because it assumes all components are registered
    // in the world.
    assert(static_cast<size_t>(component.getId()) == components_.size() - 1);
    // TODO: Page size has to be configurable at some point.
    componentPools_.emplace_back(component.getStruct().getSize());
    componentNames_.emplace(component.getName(), component.getId());
}

const Component& World::getComponent(ComponentId compId) const
{
    return components_[static_cast<size_t>(compId)];
}

void World::invokeSystem(const std::string& name, float dt)
{
    auto& system = systems_[systemNames_.at(name)];
    if (system.enabled) {
        const auto start = getTime();
        system.function(dt);
        system.lastDuration = getTime() - start;
    }
}

std::vector<World::System>& World::getSystems()
{
    return systems_;
}

void World::setSystemEnabled(const std::string& name, bool enabled)
{
    systems_[systemNames_.at(name)].enabled = enabled;
}

void World::setSystemDisabled(const std::string& name)
{
    setSystemEnabled(name, false);
}

World& getDefaultWorld()
{
    static World world;
    return world;
}

bool entityExists(EntityId id)
{
    return getDefaultWorld().entityExists(id);
}

EntityId newEntity()
{
    return getDefaultWorld().newEntity();
}

void destroyEntity(EntityId id)
{
    getDefaultWorld().destroyEntity(id);
}

std::vector<EntityId> getEntities(const ComponentMask& mask)
{
    return getDefaultWorld().getEntities(mask);
}

void registerComponent(const std::string& name, Struct&& strct)
{
    getDefaultWorld().registerComponent(name, std::forward<Struct>(strct));
}

const Component& getComponent(ComponentId compId)
{
    return getDefaultWorld().getComponent(compId);
}

const std::vector<Component>& getComponents()
{
    return getDefaultWorld().getComponents();
}

bool hasComponent(EntityId id, ComponentId compId)
{
    return getDefaultWorld().hasComponent(id, compId);
}

void removeComponent(EntityId id, ComponentId compId)
{
    getDefaultWorld().removeComponent(id, compId);
}

void setComponentEnabled(EntityId id, ComponentId compId, bool enabled)
{
    getDefaultWorld().setComponentEnabled(id, compId, enabled);
}

void setComponentDisabled(EntityId id, ComponentId compId)
{
    getDefaultWorld().setComponentDisabled(id, compId);
}

bool isComponentAllocated(EntityId id, ComponentId compId)
{
    return getDefaultWorld().isComponentAllocated(id, compId);
}

void* getComponentBuffer(EntityId id, ComponentId compId)
{
    return getDefaultWorld().getComponentBuffer(id, compId);
}

ComponentId getComponentId(const std::string& name)
{
    return getDefaultWorld().getComponentId(name);
}

void invokeSystem(const std::string& name, float dt)
{
    return getDefaultWorld().invokeSystem(name, dt);
}

std::vector<World::System>& getSystems()
{
    return getDefaultWorld().getSystems();
}

void setSystemEnabled(const std::string& name, bool enabled)
{
    getDefaultWorld().setSystemEnabled(name, enabled);
}

void setSystemDisabled(const std::string& name)
{
    getDefaultWorld().setSystemDisabled(name);
}

}