#pragma once

#include <unordered_map>

#include "ecs.hpp"
#include "fieldtype.hpp"
#include "vector_map.hpp"

namespace myl {
void loadComponents(World& world, std::string_view path);
void loadComponents(const std::string& path);
}
