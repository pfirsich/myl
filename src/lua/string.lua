R"luastring"--(

local ffi = require("ffi")

local max = math.max

ffi.cdef [[
typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} MylString;
]]

local str_type = ffi.typeof("MylString")
local str = {}

function str.set(s, luaString)
    local len = luaString:len()
    if s.capacity < len + 1 then
        ffi.C.free(s.data)
        local allocSize = max(32, max(tonumber(s.capacity * 2), len + 1))
        s.data = ffi.C.malloc(allocSize)
        s.capacity = allocSize
    end
    ffi.copy(s.data, luaString, len)
    s.data[len] = 0
    s.size = len
end

function str.str(s)
    return ffi.string(s.data, s.size)
end

local str_mt = {
    __index = str,
    __tostring = str.str
}

ffi.metatype(str_type, str_mt)

--)luastring"--"
