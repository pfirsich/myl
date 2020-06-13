#include "input.hpp"

#include <SFML/Window.hpp>
#include <boost/container/flat_map.hpp>

namespace myl {
namespace modules {
    namespace input {
        namespace {
            const boost::container::flat_map<std::string, Key> keyMap {
                { "a", Key::A },
                { "b", Key::B },
                { "c", Key::C },
                { "d", Key::D },
                { "e", Key::E },
                { "f", Key::F },
                { "g", Key::G },
                { "h", Key::H },
                { "i", Key::I },
                { "j", Key::J },
                { "k", Key::K },
                { "l", Key::L },
                { "m", Key::M },
                { "n", Key::N },
                { "o", Key::O },
                { "p", Key::P },
                { "q", Key::Q },
                { "r", Key::R },
                { "s", Key::S },
                { "t", Key::T },
                { "u", Key::U },
                { "v", Key::V },
                { "w", Key::W },
                { "x", Key::X },
                { "y", Key::Y },
                { "z", Key::Z },
                { "num0", Key::Num0 },
                { "num1", Key::Num1 },
                { "num2", Key::Num2 },
                { "num3", Key::Num3 },
                { "num4", Key::Num4 },
                { "num5", Key::Num5 },
                { "num6", Key::Num6 },
                { "num7", Key::Num7 },
                { "num8", Key::Num8 },
                { "num9", Key::Num9 },
                { "escape", Key::Escape },
                { "lctrl", Key::LCtrl },
                { "lshift", Key::LShift },
                { "lalt", Key::LAlt },
                { "lsystem", Key::LSystem },
                { "rctrl", Key::RCtrl },
                { "rshift", Key::RShift },
                { "ralt", Key::RAlt },
                { "rsystem", Key::RSystem },
                { "menu", Key::Menu },
                { "lbracket", Key::LBracket },
                { "rbracket", Key::RBracket },
                { "semicolon", Key::Semicolon },
                { "comma", Key::Comma },
                { "period", Key::Period },
                { "quote", Key::Quote },
                { "slash", Key::Slash },
                { "backslash", Key::Backslash },
                { "tilde", Key::Tilde },
                { "equal", Key::Equal },
                { "hyphen", Key::Hyphen },
                { "space", Key::Space },
                { "enter", Key::Enter },
                { "backspace", Key::Backspace },
                { "tab", Key::Tab },
                { "pageup", Key::Pageup },
                { "pagedown", Key::Pagedown },
                { "end", Key::End },
                { "home", Key::Home },
                { "insert", Key::Ins },
                { "delete", Key::Del },
                { "add", Key::Add },
                { "subtract", Key::Subtract },
                { "multiply", Key::Multiply },
                { "divide", Key::Divide },
                { "left", Key::Left },
                { "right", Key::Right },
                { "up", Key::Up },
                { "down", Key::Down },
                { "np0", Key::Np0 },
                { "np1", Key::Np1 },
                { "np2", Key::Np2 },
                { "np3", Key::Np3 },
                { "np4", Key::Np4 },
                { "np5", Key::Np5 },
                { "np6", Key::Np6 },
                { "np7", Key::Np7 },
                { "np8", Key::Np8 },
                { "np9", Key::Np9 },
                { "f1", Key::F1 },
                { "f2", Key::F2 },
                { "f3", Key::F3 },
                { "f4", Key::F4 },
                { "f5", Key::F5 },
                { "f6", Key::F6 },
                { "f7", Key::F7 },
                { "f8", Key::F8 },
                { "f9", Key::F9 },
                { "f10", Key::F10 },
                { "f11", Key::F11 },
                { "f12", Key::F12 },
                { "f13", Key::F13 },
                { "f14", Key::F14 },
                { "f15", Key::F15 },
                { "pause", Key::Pause },
            };

            constexpr auto maxKey = static_cast<size_t>(Key::Last);

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
