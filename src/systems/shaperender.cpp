#include "shaperender.hpp"

namespace c = myl::components;

namespace myl {
RectangleRenderSystem::RectangleRenderSystem()
    : ShapeRenderSystem("RectangleRender")
{
}

void RectangleRenderSystem::updateShape(myl::EntityId entity, sf::RectangleShape& shape) const
{
    static const auto cRectangleRender = myl::getComponentId("RectangleRender");
    const auto rectangleRender = myl::getComponent<c::RectangleRender>(entity, cRectangleRender);
    shape.setSize(sf::Vector2f(rectangleRender->size.x, rectangleRender->size.y));
}

CircleRenderSystem::CircleRenderSystem()
    : ShapeRenderSystem("CircleRender")
{
}

void CircleRenderSystem::updateShape(myl::EntityId entity, sf::CircleShape& shape) const
{
    static const auto cCircleRender = myl::getComponentId("CircleRender");
    const auto circleRender = myl::getComponent<c::CircleRender>(entity, cCircleRender);
    shape.setRadius(circleRender->radius);
    if (shape.getPointCount() != circleRender->pointCount)
        shape.setPointCount(circleRender->pointCount);
}
}
