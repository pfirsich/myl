#include "color.hpp"

namespace myl {

Color::Color()
    : Color(0.f, 0.f, 0.f, 1.f)
{
}

Color::Color(float r, float g, float b)
    : Color(r, g, b, 1.f)
{
}

Color::Color(float r, float g, float b, float a)
    : r(r)
    , g(g)
    , b(b)
    , a(a)
{
}

Color::Color(const glm::vec4& v)
    : Color(v.r, v.g, v.b, v.a)
{
}

Color::Color(uint32_t value)
    : r(((value & 0xff000000) >> 24) / 255.f)
    , g(((value & 0x00ff0000) >> 16) / 255.f)
    , b(((value & 0x0000ff00) >> 8) / 255.f)
    , a(((value & 0x000000ff) >> 0) / 255.f)
{
}

Color::operator glm::vec4() const
{
    return glm::vec4(r, g, b, a);
}

Color::operator uint32_t() const
{
    return static_cast<uint32_t>(r * 255.f) << 24 | static_cast<uint32_t>(g * 255.f) << 16
        | static_cast<uint32_t>(b * 255.f) << 8 | static_cast<uint32_t>(a * 255.f);
}

}
