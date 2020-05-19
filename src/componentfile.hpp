#include <unordered_map>

#include "fieldtype.hpp"

struct StructData {
    StructType structType;
    std::string name;
    bool isComponent;
};

struct ComponentFileData {
    std::unordered_map<std::string, EnumType> enums;
    std::unordered_map<std::string, StructData> structs;
};

ComponentFileData loadComponentFromFile(std::string_view path);