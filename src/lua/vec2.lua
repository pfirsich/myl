R"luastring"--(

-- Heavily based on CPML: https://github.com/excessive/cpml

local ffi = require("ffi")

local sqrt = math.sqrt
local atan2 = math.atan2
local acos = math.acos
local cos, sin = math.cos, math.sin
local min, max = math.min, math.max

ffi.cdef [[
typedef struct {
    float x, y;
} vec2;
]]

local vec2_type = ffi.typeof("vec2")
local vec2 = {}

function vec2.new(x, y)
    return vec2_type(x, y or x)
end

-- unary operations

function vec2.clone(v)
    return vec2_type(v.x, v.y)
end

function vec2.len(v)
    return sqrt(v.x*v.x + v.y*v.y)
end

function vec2.len2(v)
    return v.x*v.x + v.y*v.y
end

function vec2.normalized(v)
    if v:isZero() then
        return vec2_type(0, 0)
    end
    return v:scaled(1.0 / v:len())
end

function vec2.ortho(v)
    return vec2_type(-v.y, v.x)
end

function vec2.min(v)
    return min(v.x, v.y)
end

function vec2.max(v)
    return max(v.x, v.y)
end

function vec2.toPolar(v)
    return atan2(v.y, v.x), v:len()
end

function vec2.isVec2(v)
    return type(v) == "cdata" and ffi.istype("vec2", v)
end

function vec2.isZero(v)
    return v.x == 0 and v.y == 0
end

function vec2.toString(v)
    return string.format("(%+0.3f,%+0.3f)", v.x, v.y)
end

function vec2.unpack(a)
    return a.x, a.y
end

-- binary operations with scalar

function vec2.scaled(v, s)
    return vec2_type(v.x * s, v.y * s)
end

function vec2.limit(v, maxLen)
    local len2 = v:len2()
    if len2 > maxLen*maxLen then
        return v:scaled(maxLen / sqrt(len2))
    end
    return v:clone()
end

function vec2.rotated(v, angle)
    local c, s = cos(angle), sin(angle)
    return vec2_type(c * v.x - s * v.y,
                     s * v.x + c * v.y)
end

-- binary operations with another vec2

function vec2.add(a, b)
    return vec2_type(a.x + b.x, a.y + b.y)
end

function vec2.sub(a, b)
    return vec2_type(a.x - b.x, a.y - b.y)
end

function vec2.mul(a, b)
    return vec2_type(a.x * b.x, a.y * b.y)
end

function vec2.div(a, b)
    return vec2_type(a.x / b.x, a.y / b.y)
end

function vec2.dot(a, b)
    return a.x*b.x + a.y*b.y
end

function vec2.cross(a, b)
    return a.x*b.y - a.y*b.x
end

function vec2.dist(a, b)
    local dx = a.x - b.x
    local dy = a.y - b.y
    return sqrt(dx*dx + dy*dy)
end

function vec2.dist2(a, b)
    local dx = a.x - b.x
    local dy = a.y - b.y
    return dx*dx + dy*dy
end

function vec2.angleBetween(a, b)
    return acos(a:dot(b) / (a:len() * b:len()))
end

-- everything else

function vec2.fromPolar(angle, radius)
    radius = radius or 1.0
    return vec2_type(cos(angle) * radius, sin(angle) * radius)
end

function vec2.lerp(a, b, t)
    return vec2_type(a.x + (b.x - a.x) * t,
                     a.y + (b.y - a.y) * t)
end

-- metatable

local vec2_mt = {
    __index = vec2,
    __tostring = vec2.toString,
}

function vec2_mt.__unm(a)
    return vec2_type(-a.x, -a.y)
end

function vec2_mt.__eq(a, b)
    return vec2.isVec2(a) and vec2.isVec2(b) and a.x == b.x and a.y == b.y
end

function vec2_mt.__add(a, b)
    return a:add(b)
end

function vec2_mt.__sub(a, b)
    return a:sub(b)
end

function vec2_mt.__mul(a, b)
    if type(b) == "number" then
        return a:scaled(b)
    end
    return a:mul(b)
end

function vec2_mt.__div(a, b)
    if type(b) == "number" then
        return a:scaled(1.0 / b)
    end
    return a:div(b)
end

ffi.metatype(vec2_type, vec2_mt)
myl.vec2 = vec2_type

--)luastring"--"
