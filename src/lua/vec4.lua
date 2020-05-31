R"luastring"--(

-- Heavily based on CPML: https://github.com/excessive/cpml

local ffi = require("ffi")

local sqrt = math.sqrt
local acos = math.acos
local min, max = math.min, math.max

ffi.cdef [[
typedef struct {
    float x, y, z, w;
} vec4;
]]

local vec4_type = ffi.typeof("vec4")
local vec4 = {}

function vec4.new(x, y, z, w)
    return vec4_type(x, y, z, w)
end

-- unary operations

function vec4.clone(v)
    return vec4_type(v.x, v.y, v.z, v.w)
end

function vec4.len(v)
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w)
end

function vec4.len2(v)
    return v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w
end

function vec4.normalized(v)
    if v:isZero() then
        return vec4_type(0, 0, 0, 0)
    end
    return v:scaled(1.0 / v:len())
end

function vec4.min(v)
    return min(v.x, v.y, v.z, v.w)
end

function vec4.max(v)
    return max(v.x, v.y, v.z, v.w)
end

function vec4.isVec4(v)
    return type(v) == "cdata" and ffi.istype("vec4", v)
end

function vec4.isZero(v)
    return v.x == 0 and v.y == 0 and v.z == 0 and v.w == 0
end

function vec4.toString(v)
    return string.format("(%+0.3f,%+0.3f,%+0.3f,%+0.3f)", v.x, v.y, v.z, v.w)
end

function vec4.unpack(a)
    return a.x, a.y, a.z, a.w
end

-- binary operations with scalar

function vec4.scaled(v, s)
    return vec4_type(v.x * s, v.y * s, v.z * s, v.w * s)
end

function vec4.limit(v, maxLen)
    local len2 = v:len2()
    if len2 > maxLen*maxLen then
        return v:scaled(maxLen / sqrt(len2))
    end
    return v:clone()
end


-- binary operations with another vec4

function vec4.add(a, b)
    return vec4_type(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w)
end

function vec4.sub(a, b)
    return vec4_type(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w)
end

function vec4.mul(a, b)
    return vec4_type(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w)
end

function vec4.div(a, b)
    return vec4_type(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w)
end

function vec4.dot(a, b)
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w
end

function vec4.dist(a, b)
    return sqrt(a:dist2(b))
end

function vec4.dist2(a, b)
    local dx = a.x - b.x
    local dy = a.y - b.y
    local dz = a.z - b.z
    local dw = a.w - b.w
    return dx*dx + dy*dy + dz*dz + dw*dw
end

function vec4.angleBetween(a, b)
    return acos(a:dot(b) / (a:len() * b:len()))
end

-- everything else

function vec4.lerp(a, b, t)
    return vec4_type(a.x + (b.x - a.x) * t,
                     a.y + (b.y - a.y) * t,
                     a.z + (b.z - a.z) * t,
                     a.w + (b.w - a.w) * t)
end

-- metatable

local vec4_mt = {
    __index = vec4,
    __tostring = vec4.toString,
}

function vec4_mt.__call(_, x, y, z, w)
    return vec4_type(x, y, z, w)
end

function vec4_mt.__unm(a)
    return vec4_type(-a.x, -a.y, -a.z, -a.w)
end

function vec4_mt.__eq(a, b)
    return vec4.isVec4(a) and vec4.isVec4(b) and a.x == b.x and a.y == b.y and a.z == b.z and a.w == b.w
end

function vec4_mt.__add(a, b)
    return a:add(b)
end

function vec4_mt.__sub(a, b)
    return a:sub(b)
end

function vec4_mt.__mul(a, b)
    if type(b) == "number" then
        return a:scaled(b)
    end
    return a:mul(b)
end

function vec4_mt.__div(a, b)
    if type(b) == "number" then
        return a:scaled(1.0 / b)
    end
    return a:div(b)
end

ffi.metatype(vec4_type, vec4_mt)
myl.vec4 = vec4_type

--)luastring"--"
