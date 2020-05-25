#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "../componentfile.hpp"
#include "../ecs.hpp"

namespace Lua {
void init(sol::state& lua, World& world);
}
