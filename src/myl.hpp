#pragma once

#include <string>
#include <vector>

#include "components.hpp"
#include "ecs.hpp"
#include "systems.hpp"
#include "util.hpp"

namespace myl {
namespace c = components;

bool initGameDirectory(const std::vector<std::string>& args);
}
