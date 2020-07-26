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

struct RectangleRenderSystem : public myl::RegisteredSystem<RectangleRenderSystem> {
    inline static const std::string name = "RectangleRender";

    void update(float dt);
};

struct CircleRenderSystem : public myl::RegisteredSystem<CircleRenderSystem> {
    inline static const std::string name = "CircleRender";

    void update(float dt);
};
}
