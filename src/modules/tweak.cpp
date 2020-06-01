#include "tweak.hpp"

#include <algorithm>
#include <fstream>

#include <toml++/toml.h>

namespace myl {
namespace modules {
    namespace tweak {
        namespace detail {
            Registry& getRegistry()
            {
                static Registry registry;
                return registry;
            }
        }

        using namespace detail;

        template <typename T>
        static bool valueModified(const T& val)
        {
            return val.current != val.initial;
        }

        void save()
        {
            toml::table tbl;
            bool anyModified = false;
            for (const auto& [name, var] : getRegistry()) {
                const bool modified
                    = std::visit([](auto&& arg) { return arg.current != arg.initial; }, var);
                anyModified = anyModified || modified;
                if (modified) {
                    if (std::holds_alternative<ValuePair<int>>(var)) {
                        const auto v = std::get<ValuePair<int>>(var).current;
                        tbl.insert(name, toml::value(v));
                    } else if (std::holds_alternative<ValuePair<float>>(var)) {
                        const auto v = std::get<ValuePair<float>>(var).current;
                        tbl.insert(name, toml::value(v));
                    } else if (std::holds_alternative<ValuePair<glm::vec2>>(var)) {
                        const auto v = std::get<ValuePair<glm::vec2>>(var).current;
                        tbl.insert(name, toml::array(v.x, v.y));
                    } else if (std::holds_alternative<ValuePair<glm::vec3>>(var)) {
                        const auto v = std::get<ValuePair<glm::vec3>>(var).current;
                        tbl.insert(name, toml::array(v.x, v.y, v.z));
                    } else if (std::holds_alternative<ValuePair<glm::vec4>>(var)) {
                        const auto v = std::get<ValuePair<glm::vec4>>(var).current;
                        tbl.insert(name, toml::array(v.x, v.y, v.z, v.w));
                    } else if (std::holds_alternative<ValuePair<std::string>>(var)) {
                        const auto v = std::get<ValuePair<std::string>>(var).current;
                        tbl.insert(name, toml::value(v));
                    } else if (std::holds_alternative<ValuePair<Color>>(var)) {
                        const auto v = std::get<ValuePair<Color>>(var).current;
                        tbl.insert(name, toml::array(v.r, v.g, v.b, v.a));
                    }
                }
            }
            if (anyModified) {
                std::ofstream os("tweaks.toml");
                os << tbl;
            }
        }

        std::vector<std::pair<std::string, size_t>> getTweaks()
        {
            std::vector<std::pair<std::string, size_t>> tweaks;
            for (const auto& [name, var] : getRegistry())
                tweaks.emplace_back(name, var.index());
            return tweaks;
        }

        namespace shortcuts {
            int Ti(const std::string& name)
            {
                return T<int>(name);
            }

            float Tf(const std::string& name)
            {
                return T<float>(name);
            }

            glm::vec2 T2(const std::string& name)
            {
                return T<glm::vec2>(name);
            }

            glm::vec3 T3(const std::string& name)
            {
                return T<glm::vec3>(name);
            }

            glm::vec4 T4(const std::string& name)
            {
                return T<glm::vec4>(name);
            }

            std::string Ts(const std::string& name)
            {
                return T<std::string>(name);
            }

            Color Tc(const std::string& name)
            {
                return T<Color>(name);
            }
        }
    }
}
}
