#include "input.hpp"

#include <SFML/Window.hpp>
#include <boost/container/flat_map.hpp>

namespace myl {
namespace modules {
    namespace input {
        namespace {
            const boost::container::flat_map<std::string, Key> keyMap {
                { "a", Key::a },
                { "b", Key::b },
                { "c", Key::c },
                { "d", Key::d },
                { "e", Key::e },
                { "f", Key::f },
                { "g", Key::g },
                { "h", Key::h },
                { "i", Key::i },
                { "j", Key::j },
                { "k", Key::k },
                { "l", Key::l },
                { "m", Key::m },
                { "n", Key::n },
                { "o", Key::o },
                { "p", Key::p },
                { "q", Key::q },
                { "r", Key::r },
                { "s", Key::s },
                { "t", Key::t },
                { "u", Key::u },
                { "v", Key::v },
                { "w", Key::w },
                { "x", Key::x },
                { "y", Key::y },
                { "z", Key::z },
                { "num0", Key::num0 },
                { "num1", Key::num1 },
                { "num2", Key::num2 },
                { "num3", Key::num3 },
                { "num4", Key::num4 },
                { "num5", Key::num5 },
                { "num6", Key::num6 },
                { "num7", Key::num7 },
                { "num8", Key::num8 },
                { "num9", Key::num9 },
                { "escape", Key::escape },
                { "lctrl", Key::lctrl },
                { "lshift", Key::lshift },
                { "lalt", Key::lalt },
                { "lsystem", Key::lsystem },
                { "rctrl", Key::rctrl },
                { "rshift", Key::rshift },
                { "ralt", Key::ralt },
                { "rsystem", Key::rsystem },
                { "menu", Key::menu },
                { "lbracket", Key::lbracket },
                { "rbracket", Key::rbracket },
                { "semicolon", Key::semicolon },
                { "comma", Key::comma },
                { "period", Key::period },
                { "quote", Key::quote },
                { "slash", Key::slash },
                { "backslash", Key::backslash },
                { "tilde", Key::tilde },
                { "equal", Key::equal },
                { "hyphen", Key::hyphen },
                { "space", Key::space },
                { "enter", Key::enter },
                { "backspace", Key::backspace },
                { "tab", Key::tab },
                { "pageup", Key::pageup },
                { "pagedown", Key::pagedown },
                { "end", Key::end },
                { "home", Key::home },
                { "insert", Key::ins },
                { "delete", Key::del },
                { "add", Key::add },
                { "subtract", Key::subtract },
                { "multiply", Key::multiply },
                { "divide", Key::divide },
                { "left", Key::left },
                { "right", Key::right },
                { "up", Key::up },
                { "down", Key::down },
                { "np0", Key::np0 },
                { "np1", Key::np1 },
                { "np2", Key::np2 },
                { "np3", Key::np3 },
                { "np4", Key::np4 },
                { "np5", Key::np5 },
                { "np6", Key::np6 },
                { "np7", Key::np7 },
                { "np8", Key::np8 },
                { "np9", Key::np9 },
                { "f1", Key::f1 },
                { "f2", Key::f2 },
                { "f3", Key::f3 },
                { "f4", Key::f4 },
                { "f5", Key::f5 },
                { "f6", Key::f6 },
                { "f7", Key::f7 },
                { "f8", Key::f8 },
                { "f9", Key::f9 },
                { "f10", Key::f10 },
                { "f11", Key::f11 },
                { "f12", Key::f12 },
                { "f13", Key::f13 },
                { "f14", Key::f14 },
                { "f15", Key::f15 },
                { "pause", Key::pause },
            };

            constexpr auto maxKey = static_cast<size_t>(Key::last);

            std::vector<uint8_t>& currentState()
            {
                static std::vector<uint8_t> s(maxKey, false);
                return s;
            }

            std::vector<uint8_t>& lastDown()
            {
                static std::vector<uint8_t> s(maxKey, false);
                return s;
            }
        }

        namespace internal {
            void setState(Key key, bool state)
            {
                currentState()[static_cast<size_t>(key)] = state;
            }

            void saveLastState()
            {
                auto& ld = lastDown();
                for (size_t i = 0; i < maxKey; ++i) {
                    ld[i] = getKeyboardDown(static_cast<Key>(i));
                }
            }
        }

        bool getLastDown(Key key)
        {
            return lastDown()[static_cast<size_t>(key)];
        }

        bool getKeyboardDown(Key key)
        {
            return currentState()[static_cast<size_t>(key)];
        }

        bool getKeyboardDown(const std::string& keyName)
        {
            return getKeyboardDown(keyMap.at(keyName));
        }

        bool getKeyboardPressed(Key key)
        {
            return getKeyboardDown(key) && !getLastDown(key);
        }

        bool getKeyboardPressed(const std::string& keyName)
        {
            return getKeyboardPressed(keyMap.at(keyName));
        }

        bool getKeyboardReleased(Key key)
        {
            return !getKeyboardDown(key) && getLastDown(key);
        }

        bool getKeyboardReleased(const std::string& keyName)
        {
            return getKeyboardReleased(keyMap.at(keyName));
        }
    }
}
}