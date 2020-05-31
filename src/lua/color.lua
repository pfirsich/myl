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

local colorCache = {}

-- Creates a new color, can be called like:
-- color() -> black
-- color(1, 1, 1) -> white
-- color(1, 1, 1, 0.5) -> white 50% opacity
-- color("#ff0000") -> red
-- color("#ff000099") -> red 60% opacity
-- color("#f00") -> red
-- color("#f00009") -> red 60% opacity
function color.new(r, g, b, a)
    if r == nil then
        return color_type(0, 0, 0, 1)
    elseif type(r) == "string" then
        local hex = r

        if colorCache[hex] then
            return colorCache[hex]:clone()
        end

        local len = hex:len()
        if len == 7 or len == 9 then
            r = tonumber(hex:sub(2, 3), 0x10) / 0xff
            g = tonumber(hex:sub(4, 5), 0x10) / 0xff
            b = tonumber(hex:sub(6, 7), 0x10) / 0xff
            a = len == 9 and tonumber(hex:sub(8, 9), 0x10) / 0xff or 1.0
        elseif len == 4 or len == 5 then
            r = tonumber(hex:sub(2, 2), 0x10) * 0x11 / 0xff
            g = tonumber(hex:sub(3, 3), 0x10) * 0x11 / 0xff
            b = tonumber(hex:sub(4, 4), 0x10) * 0x11 / 0xff
            a = len == 5 and tonumber(hex:sub(5, 5), 0x10) * 0x11 / 0xff or 1.0
        else
            error("Color hex string needs to be 4, 5, 7 or 9 characters long")
        end

        local value = color_type(r, g, b, a)
        colorCache[hex] = value:clone()
        return value
    end

    return color_type(r, g, b, a or 1.0)
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
    return string.format("Color (%+0.3f,%+0.3f,%+0.3f,%+0.3f)", v.r, v.g, v.b, v.a)
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
    return color.new(x, y)
end

function color_mt.__eq(a, b)
    return color.isColor(a) and color.isColor(b) and a.r == b.r and a.b == b.b and a.g == b.g and a.a == b.a
end

ffi.metatype(color_type, color_mt)
myl.color = color_type

--)luastring"--"
