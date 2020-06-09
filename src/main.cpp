#include "lua/lua.hpp"
#include "myl.hpp"

int main(int argc, char** argv)
{
    const auto args = std::vector<std::string> { argv + 1, argv + argc };

    if (!myl::initGameDirectory(args)) {
        std::cerr << "Game directory not found." << std::endl;
        return EXIT_FAILURE;
    }

    myl::registerBuiltinComponents();

    myl::registerBuiltinSystems();

    myl::lua::State lua;
    lua.init();
    if (lua.hasMain()) {
        if (!lua.executeMain())
            return EXIT_FAILURE;
    } else {
        std::cerr << "No main entry point found." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
