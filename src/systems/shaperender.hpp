#pragma once

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

#include "components.hpp"
#include "ecs.hpp"
#include "modules/window.hpp"

namespace myl {
namespace components {
    struct RectangleRender {
        glm::vec2 size;
    };

    struct CircleRender {
        float radius;
        size_t pointCount;
    };
}

template <typename Derived, typename Shape>
class ShapeRenderSystem {
public:
    ShapeRenderSystem(const std::string& componentName)
        : componentId_(myl::getComponentId(componentName))
        , shapes_(componentId_)
    {
    }

    void update(float /*dt*/)
    {
        static const auto cTransform = myl::getComponentId("Transform");
        for (auto entity : myl::getEntities(cTransform + componentId_)) {
            auto& shape = shapes_.template get<true>(entity);
            const auto trafo = myl::getComponent<components::Transform>(entity, cTransform);
            shape.setPosition(trafo->position.x, trafo->position.y);
            shape.setRotation(trafo->angle / M_PI * 180.0f);
            shape.setOrigin(trafo->origin.x, trafo->origin.y);
            shape.setScale(trafo->scale.x, trafo->scale.y);
            static_cast<Derived*>(this)->updateShape(entity, shape);
            shape.setFillColor(getColor(entity));
            modules::window::getWindow().draw(shape);
        }
        shapes_.remove();
    }

private:
    static sf::Color getColor(myl::EntityId entity)
    {
        static const auto cColor = myl::getComponentId("Color");
        if (myl::hasComponent(entity, cColor)) {
            auto& color = myl::getComponent<components::Color>(entity, cColor)->value;
            return sf::Color(static_cast<uint32_t>(color));
        } else {
            return sf::Color(255, 255, 255, 255);
        }
    }

    myl::ComponentId componentId_;
    myl::SystemData<Shape> shapes_;
};

struct RectangleRenderSystem : public myl::RegisteredSystem<RectangleRenderSystem>,
                               public ShapeRenderSystem<RectangleRenderSystem, sf::RectangleShape> {
    inline static const std::string name = "RectangleRender";

    RectangleRenderSystem();

    void updateShape(myl::EntityId entity, sf::RectangleShape& shape) const;
};

struct CircleRenderSystem : public myl::RegisteredSystem<CircleRenderSystem>,
                            public ShapeRenderSystem<CircleRenderSystem, sf::CircleShape> {
    inline static const std::string name = "CircleRender";

    CircleRenderSystem();

    void updateShape(myl::EntityId entity, sf::CircleShape& shape) const;
};
}
