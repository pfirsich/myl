#pragma once

#include <glm/glm.hpp>

#include "struct.hpp"

namespace myl {
namespace components {
    struct Name {
        myl::String value;
    };

    struct Transform {
        glm::vec2 position;
        float angle;
        glm::vec2 scale;
        glm::vec2 origin;

        glm::vec2 apply(const glm::vec2& point) const;
    };

    struct Color {
        myl::Color value;
    };
}

void registerBuiltinComponents();
}
