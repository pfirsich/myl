#pragma once

#include <fmt/format.h>
#include <glm/glm.hpp>

template <>
struct fmt::formatter<glm::vec2> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::vec2& v, FormatContext& ctx)
    {
        return format_to(ctx.out(), "({:.3f}, {:.3f})", v.x, v.y);
    }
};

template <>
struct fmt::formatter<glm::vec3> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::vec3& v, FormatContext& ctx)
    {
        return format_to(ctx.out(), "({:.3f}, {:.3f}, {:.3f})", v.x, v.y, v.z);
    }
};

template <>
struct fmt::formatter<glm::vec4> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const glm::vec4& v, FormatContext& ctx)
    {
        return format_to(ctx.out(), "({:.3f}, {:.3f}, {:.3f}, {:.3f})", v.x, v.y, v.z, v.w);
    }
};
