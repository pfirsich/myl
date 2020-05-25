#include "ecs.hpp"

#include <iostream>

#include "struct.hpp"

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
    return std::pair<size_t, size_t>(entityId / pageSize_, entityId % pageSize_);
}

void* ComponentPool::getPointer(size_t page, size_t index)
{
    assert(pages_[page].data);
    return reinterpret_cast<uint8_t*>(pages_[page].data) + componentSize_ * index;
}

Component::Component(const std::string& name, Struct&& s)
    : id_(getComponentId())
    , name_(name)
    , struct_(s)
{
}

Component::Id Component::getId() const
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

Component::Id Component::getComponentId()
{
    static Id counter = 0;
    assert(counter < maxComponents);
    return counter++;
}

ComponentMask::ComponentMask(Component::Id id)
{
    mask_.set(id, true);
}

bool ComponentMask::includes(Component::Id id) const
{
    return mask_.test(id);
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

void ComponentMask::include(Component::Id id)
{
    mask_.set(id, true);
}

ComponentMask ComponentMask::operator+(Component::Id id) const
{
    ComponentMask mask(*this);
    mask.mask_.set(id, true);
    return mask;
}

ComponentMask ComponentMask::operator+(const ComponentMask& other) const
{
    return ComponentMask(mask_ | other.mask_);
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

const std::vector<Component>& World::getComponents()
{
    return components_;
}

bool World::entityExists(EntityId id) const
{
    return id < entities_.size() && entities_[id].exists;
}

EntityId World::newEntity()
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

void World::destroyEntity(EntityId id)
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

bool World::hasComponent(EntityId id, Component::Id compId)
{
    assert(entityExists(id));
    return entities_[id].components.includes(compId);
}

void World::removeComponent(EntityId id, Component::Id compId)
{
    componentPools_[compId].remove(id);
}

Component::Id World::getComponentId(const std::string& name) const
{
    return componentNames_.at(name);
}

// Implement foreachEntity in the future that returns a custom iterator.
std::vector<EntityId> World::getEntities(const ComponentMask& mask) const
{
    std::vector<EntityId> ids;
    for (EntityId id = 0; id < entities_.size(); ++id) {
        auto& entity = entities_[id];
        if (entity.exists && entity.components.includes(mask))
            ids.push_back(id);
    }
    return ids;
}

void World::invokeSystem(const std::string& name, float dt)
{
    systems_.at(name).function(dt);
}
