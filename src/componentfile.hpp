#pragma once

#include <unordered_map>

#include "fieldtype.hpp"
#include "vector_map.hpp"

struct StructData {
    StructType structType;
    std::string name;
    bool isComponent;
};

struct ComponentFileData {
    vector_map<std::string, EnumType> enums;
    vector_map<std::string, StructData> structs;
};

ComponentFileData loadComponentFromFile(std::string_view path);