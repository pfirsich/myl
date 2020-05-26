#include "ecs.hpp"

#include <iostream>

#include "util.hpp"

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
    return pages_.size() > page && pages_[page].occupied[index];
}

void* ComponentPool::add(EntityId entityId)
{
    assert(!has(entityId));
    const auto [page, index] = getIndices(entityId);

    if (page >= pages_.size())
        pages_.resize(page + 1, Page(pageSize_));

    auto& pageObj = pages_[page];
    if (!pageObj.data)
        pageObj.data = ::operator new(pageSize_* componentSize_);
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

    if (pageObj.occupied.none()) {
        ::operator delete(pageObj.data);
        pageObj.data = nullptr;
    }
}

ComponentPool::Page::Page(size_t pageSize)
    : occupied(pageSize)
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
    return reinterpret_cast<uint8_t*>(pages_[page].data) + componentSize_ * index;
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
    entities_[static_cast<size_t>(id)].components -= compId;
    componentPools_[static_cast<size_t>(compId)].remove(id);
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
