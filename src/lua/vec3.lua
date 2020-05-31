R"luastring"--(

-- Heavily based on CPML: https://github.com/excessive/cpml

local ffi = require("ffi")

local sqrt = math.sqrt
local acos = math.acos
local min, max = math.min, math.max

ffi.cdef [[
typedef struct {
    float x, y, z;
} vec3;
]]

local vec3_type = ffi.typeof("vec3")
local vec3 = {}

function vec3.new(x, y, z)
    return vec3_type(x, y, z)
end

-- unary operations

function vec3.clone(v)
    return vec3_type(v.x, v.y, v.z)
end

function vec3.len(v)
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z)
end

function vec3.len2(v)
    return v.x*v.x + v.y*v.y + v.z*v.z
end

function vec3.normalized(v)
    if v:isZero() then
        return vec3_type(0, 0, 0)
    end
    return v:scaled(1.0 / v:len())
end

function vec3.min(v)
    return min(v.x, v.y, v.z)
end

function vec3.max(v)
    return max(v.x, v.y, v.z)
end

function vec3.isVec3(v)
    return type(v) == "cdata" and ffi.istype("vec3", v)
end

function vec3.isZero(v)
    return v.x == 0 and v.y == 0 and v.z == 0
end

function vec3.toString(v)
    return string.format("(%+0.3f,%+0.3f,%+0.3f)", v.x, v.y, v.z)
end

function vec3.unpack(a)
    return a.x, a.y, a.z
end

-- binary operations with scalar

function vec3.scaled(v, s)
    return vec3_type(v.x * s, v.y * s, v.z * s)
end

function vec3.limit(v, maxLen)
    local len2 = v:len2()
    if len2 > maxLen*maxLen then
        return v:scaled(maxLen / sqrt(len2))
    end
    return v:clone()
end


-- binary operations with another vec3

function vec3.add(a, b)
    return vec3_type(a.x + b.x, a.y + b.y, a.z + b.z)
end

function vec3.sub(a, b)
    return vec3_type(a.x - b.x, a.y - b.y, a.z - b.z)
end

function vec3.mul(a, b)
    return vec3_type(a.x * b.x, a.y * b.y, a.z * b.z)
end

function vec3.div(a, b)
    return vec3_type(a.x / b.x, a.y / b.y, a.z / b.z)
end

function vec3.dot(a, b)
    return a.x*b.x + a.y*b.y + a.z*b.z
end

function vec3.cross(a, b)
    return vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    )
end

function vec3.dist(a, b)
    return sqrt(a:dist2(b))
end

function vec3.dist2(a, b)
    local dx = a.x - b.x
    local dy = a.y - b.y
    local dz = a.z - b.z
    return dx*dx + dy*dy + dz*dz
end

function vec3.angleBetween(a, b)
    return acos(a:dot(b) / (a:len() * b:len()))
end

-- everything else

function vec3.lerp(a, b, t)
    return vec3_type(a.x + (b.x - a.x) * t,
                     a.y + (b.y - a.y) * t,
                     a.z + (b.z - a.z) * t)
end

-- metatable

local vec3_mt = {
    __index = vec3,
    __tostring = vec3.toString,
}

function vec3_mt.__call(_, x, y, z)
    return vec3_type(x, y, z)
end

function vec3_mt.__unm(a)
    return vec3_type(-a.x, -a.y, -a.z)
end

function vec3_mt.__eq(a, b)
    return vec3.isVec3(a) and vec3.isVec3(b) and a.x == b.x and a.y == b.y and a.z == b.z
end

function vec3_mt.__add(a, b)
    return a:add(b)
end

function vec3_mt.__sub(a, b)
    return a:sub(b)
end

function vec3_mt.__mul(a, b)
    if type(b) == "number" then
        return a:scaled(b)
    end
    return a:mul(b)
end

function vec3_mt.__div(a, b)
    if type(b) == "number" then
        return a:scaled(1.0 / b)
    end
    return a:div(b)
end

ffi.metatype(vec3_type, vec3_mt)
myl.vec3 = vec3_type

--)luastring"--"
