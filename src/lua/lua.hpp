#pragma once

#include <boost/signals2.hpp>
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "../componentfile.hpp"
#include "../ecs.hpp"

namespace myl {
namespace lua {
    class State {
    public:
        State(World& world);
        State();
        ~State();

        void init();

        bool hasMain() const;
        // returns false on error
        bool executeMain();

    private:
        static void componentRegistered(sol::state& lua, const Component& component);
        static int exceptionHandler(lua_State* L,
            sol::optional<const std::exception&> maybeException, sol::string_view description);

        sol::state lua_;
        World& world_;
        std::vector<std::string> registeredSystems_;
        boost::signals2::scoped_connection connection_;
    };
}
}
