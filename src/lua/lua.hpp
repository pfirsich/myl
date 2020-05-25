#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "../componentfile.hpp"
#include "../ecs.hpp"

namespace Lua {
std::string getAsCString(const StructData& structData);

void init(sol::state& lua, const ComponentFileData& componentData, World& world);
}
