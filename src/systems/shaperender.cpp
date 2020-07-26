#include "shaperender.hpp"

#include "../batch.hpp"

namespace c = myl::components;

namespace myl {
Batch& getBatch()
{
    // Scientifically chosen, highly optimized. Can't explain, because you wouldn't understand.
    static Batch batch(4096, 4096);
    // TODO: Put this in a better place
    batch.setTransform(glm::ortho(0.0f, 1024.0f, 768.0f, 0.0f));
    return batch;
}

void RectangleRenderSystem::update(float /*dt*/)
{
    static const auto cTransform = myl::getComponentId("Transform");
    static const auto cRectangle = myl::getComponentId("RectangleRender");
    static const auto cColor = myl::getComponentId("Color");
    auto& batch = getBatch();
    for (auto entity : myl::getEntities(cTransform + cRectangle)) {
        const auto trafo = myl::getComponent<c::Transform>(entity, cTransform);
        const auto rect = myl::getComponent<c::RectangleRender>(entity, cRectangle);
        const auto color = myl::hasComponent(entity, cColor)
            ? static_cast<glm::vec4>(myl::getComponent<c::Color>(entity, cColor)->value)
            : glm::vec4(1.0f);
        batch.addRectangle(*trafo, color, rect->size);
    }
    batch.flush();
}

void CircleRenderSystem::update(float /*dt*/)
{
    static const auto cTransform = myl::getComponentId("Transform");
    static const auto cCircle = myl::getComponentId("CircleRender");
    static const auto cColor = myl::getComponentId("Color");
    auto& batch = getBatch();
    for (auto entity : myl::getEntities(cTransform + cCircle)) {
        const auto trafo = myl::getComponent<c::Transform>(entity, cTransform);
        const auto circle = myl::getComponent<c::CircleRender>(entity, cCircle);
        const auto color = myl::hasComponent(entity, cColor)
            ? static_cast<glm::vec4>(myl::getComponent<c::Color>(entity, cColor)->value)
            : glm::vec4(1.0f);
        batch.addCircle(*trafo, color, circle->radius, circle->pointCount);
    }
    batch.flush();
}
}
