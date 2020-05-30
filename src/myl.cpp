#include "myl.hpp"

#include <filesystem>

namespace fs = std::filesystem;

namespace myl {
bool initGameDirectory(const std::vector<std::string>& args)
{
    if (args.empty())
        // Use current directory!
        return true;

    const auto dir = args[0];
    if (fs::exists(dir)) {
        fs::current_path(dir);
        return true;
    }

    return false;
}
}
