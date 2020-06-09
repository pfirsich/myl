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
    };

    struct Color {
        myl::Color value;
    };
}

void registerBuiltinComponents();
}
