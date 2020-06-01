#pragma once

#include <fmt/format.h>
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

    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;

    operator glm::vec4() const;
    explicit operator uint32_t() const;
};

}

template <>
struct fmt::formatter<myl::Color> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const myl::Color& c, FormatContext& ctx)
    {
        return format_to(ctx.out(), "({:.2f}, {:.2f}, {:.2f}, {:.2f})", c.r, c.g, c.b, c.a);
    }
};
