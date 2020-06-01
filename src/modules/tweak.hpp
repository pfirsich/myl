#pragma once

#include <string>
#include <variant>
#include <vector>

#include <boost/container/flat_map.hpp>
#include <glm/glm.hpp>

#include "../color.hpp"

namespace myl {
namespace modules {
    namespace tweak {
        namespace detail {
            using boost::container::flat_map;

            template <typename T>
            struct ValuePair {
                T initial;
                T current;

                ValuePair(const T& init)
                    : initial(init)
                    , current(init)
                {
                }
            };

            using Value = std::variant<ValuePair<int>, ValuePair<float>, ValuePair<glm::vec2>,
                ValuePair<glm::vec3>, ValuePair<glm::vec4>, ValuePair<std::string>,
                ValuePair<Color>>;
            using Registry = flat_map<std::string, Value>;

            Registry& getRegistry();

            template <typename T>
            ValuePair<T>& getValuePair(const std::string& name)
            {
                auto& reg = detail::getRegistry();
                const auto it = reg.find(name);
                assert(it != reg.end() && "Unknown tweak");
                assert(
                    std::holds_alternative<detail::ValuePair<T>>(it->second) && "Wrong tweak type");
                return std::get<detail::ValuePair<T>>(it->second);
            }
        }

        enum class ValueType { int_, float_, vec2, vec3, vec4, string, color, last };
        static_assert(static_cast<size_t>(ValueType::last) == std::variant_size_v<detail::Value>);

        void save();

        template <typename T>
        T get(const std::string& name)
        {
            return detail::getValuePair<T>(name).current;
        }

        template <typename T>
        T get(const std::string& name, const T& init)
        {
            auto& reg = detail::getRegistry();
            const auto it = reg.emplace(name, detail::ValuePair<T>(init)).first;
            assert(std::holds_alternative<detail::ValuePair<T>>(it->second) && "Wrong tweak type");
            return std::get<detail::ValuePair<T>>(it->second).current;
        }

        template <typename T>
        T getInitial(const std::string& name)
        {
            return detail::getValuePair<T>(name).initial;
        }

        template <typename T>
        void set(const std::string& name, const T& val)
        {
            detail::getValuePair<T>(name).current = val;
        }

        std::vector<std::pair<std::string, size_t>> getTweaks();

        namespace shortcuts {
            template <typename U>
            U T(const std::string& name)
            {
                return get<U>(name);
            }

            int Ti(const std::string& name);
            float Tf(const std::string& name);
            glm::vec2 T2(const std::string& name);
            glm::vec3 T3(const std::string& name);
            glm::vec4 T4(const std::string& name);
            std::string Ts(const std::string& name);
            Color Tc(const std::string& name);

            template <typename U>
            U T(const std::string& name, const U& init)
            {
                return get<U>(name, init);
            }
        }
    }
}
}
