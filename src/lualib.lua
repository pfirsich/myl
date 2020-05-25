R"luastring"--(

local ffi = require("ffi")

ffi.cdef [[
typedef struct {
    float x, y;
} vec2;

typedef struct {
    float x, y, z;
} vec3;

typedef struct {
    float x, y, z, w;
} vec4;
]]

local vec2_new = ffi.typeof("vec2")
local vec2_methods = {}
local vec2_mt = {
    __index = vec2_methods
}

function vec2_methods.add(a, b)
    return vec2_new(a.x + b.x, a.y + b.y)
end

function vec2_methods.mul(a, b)
    return vec2_new(a.x * b.x, a.y * b.y)
end

function vec2_methods.scale(a, b)
    return vec2_new(a.x * b, a.y * b)
end

function vec2_mt.__add(a, b)
    return a:add(b)
end

function vec2_mt.__mul(a, b)
    if type(b) == "number" then
        return a:scale(b)
    end
    return a:mul(b)
end

ffi.metatype(vec2_new, vec2_mt)
myl.vec2 = vec2_new

myl.c = {}
myl._componentTypes = {}

function myl.addComponent(entityId, component)
    return ffi.cast(myl._componentTypes[component], myl._addComponent(entityId, component))[0]
end

function myl.getComponent(entityId, component)
    return ffi.cast(myl._componentTypes[component], myl._getComponent(entityId, component))[0]
end

function myl.getComponents(entityId, component, ...)
    if select("#", ...) == 0 then
        return myl.getComponent(entityId, component)
    else
        return myl.getComponent(entityId, component), myl.getComponents(entityId, ...)
    end
end

function myl.foreachEntityFFF(...)
    local buffer, count = myl._getEntityList(...)
    local array = ffi.cast("uint32_t*", buffer)
    local i = 0
    return function()
        if i < count then
            local entity = array[i]
            i = i + 1
            return entity
        else
            myl._freeEntityList(buffer)
            return nil
        end
    end
end

--)luastring"--"