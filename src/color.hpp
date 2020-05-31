#pragma once

#include <glm/glm.hpp>

namespace myl {

struct Color {
    float r, g, b, a;

    Color();
    Color(float r, float g, float b);
    Color(float r, float g, float b, float a);
    Color(uint32_t value);
    Color(const glm::vec4& v);
    ~Color() = default;

    Color(const Color& other) = default;
    Color(Color&& other) = default;
    Color& operator=(const Color& other) = default;
    Color& operator=(Color&& other) = default;

    operator glm::vec4() const;
    explicit operator uint32_t() const;
};

}
