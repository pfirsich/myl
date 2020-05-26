R"luastring"--(

local ffi = require("ffi")

ffi.cdef [[
typedef struct {
    float x, y, z;
} vec3;

typedef struct {
    float x, y, z, w;
} vec4;

void* malloc(size_t size);
void free(void *ptr);
]]

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

--)luastring"--"
