#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "../componentfile.hpp"
#include "../ecs.hpp"

namespace myl {
namespace Lua {
    // No world parameter => lua has only access to default world (for now)
    void init(sol::state& lua);
}
}
