#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "../componentfile.hpp"
#include "../ecs.hpp"

namespace myl {
namespace lua {
    class State {
    public:
        void init(myl::World& world);
        void init();

        bool hasMain() const;
        // returns false on error
        bool executeMain();

    private:
        sol::state lua_;
    };
}
}
