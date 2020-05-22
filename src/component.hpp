#pragma once

#include "struct.hpp"

enum class Type {
    invalid,
    bool_,
    u8,
    i8,
    u16,
    i16,
    u32,
    i32,
    u64,
    i64,
    f32,
    vec2,
    vec3,
    vec4,
    string,

    // need metadata
    enum_,
    struct_,

    // "template types"
    array,
    vector,
    map,
};

class Component : public Struct {
};

class ComponentBuilder {
public:
    ComponentBuilder build() const;

private:
    StructBuilder builder_;
};