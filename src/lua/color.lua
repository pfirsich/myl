R"luastring"--(

-- Heavily based on CPML: https://github.com/excessive/cpml

local ffi = require("ffi")

ffi.cdef [[
typedef struct {
    float r, g, b, a;
} color;
]]

local color_type = ffi.typeof("color")
local color = {}

function color.new(r, g, b, a)
    return color_type(r, g, b, a)
end

-- unary operations

function color.clone(v)
    return color_type(v.r, v.g, v.b, v.a)
end

function color.isColor(v)
    return type(v) == "cdata" and ffi.istype("color", v)
end

function color.toVec4(v)
    return myl.vec4(v.r, v.g, v.b, v.a)
end

function color.toString(v)
    return string.format("(%+0.3f,%+0.3f,%+0.3f,%+0.3f)", v.r, v.g, v.b, v.a)
end

function color.unpack(v)
    return v.r, v.g, v.b, v.a
end

-- binary operations with another color

function color.add(a, b)
    return color_type(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a)
end

function color.sub(a, b)
    return color_type(a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a)
end

-- metatable

local color_mt = {
    __index = color,
    __tostring = color.toString,
}

function color_mt.__call(_, x, y)
    return color_type(x, y)
end

function color_mt.__eq(a, b)
    return color.isVec2(a) and color.isVec2(b) and a.x == b.x and a.y == b.y
end

ffi.metatype(color_type, color_mt)
myl.color = color_type

--)luastring"--"
