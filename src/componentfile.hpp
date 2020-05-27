#pragma once

#include <unordered_map>

#include "ecs.hpp"
#include "fieldtype.hpp"
#include "vector_map.hpp"

namespace myl {
void loadComponentsFromFile(World& world, std::string_view path);
void loadComponentsFromFile(std::string_view path);
}