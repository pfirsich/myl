#pragma once

#include <string>

namespace myl {
namespace modules {
    namespace input {
        enum class Key { // Keep compatible with sf::Keyboard::Key!
            Unknown = -1,
            A = 0,
            B,
            C,
            D,
            E,
            F,
            G,
            H,
            I,
            J,
            K,
            L,
            M,
            N,
            O,
            P,
            Q,
            R,
            S,
            T,
            U,
            V,
            W,
            X,
            Y,
            Z,
            Num0,
            Num1,
            Num2,
            Num3,
            Num4,
            Num5,
            Num6,
            Num7,
            Num8,
            Num9,
            Escape,
            LCtrl,
            LShift,
            LAlt,
            LSystem,
            RCtrl,
            RShift,
            RAlt,
            RSystem,

            Menu,
            LBracket,
            RBracket,
            Semicolon,
            Comma,
            Period,
            Quote,
            Slash,
            Backslash,
            Tilde,
            Equal,
            Hyphen,
            Space,
            Enter,
            Backspace,
            Tab,
            Pageup,
            Pagedown,
            End,
            Home,
            Ins,
            Del,
            Add,
            Subtract,
            Multiply,
            Divide,
            Left,
            Right,
            Up,
            Down,
            Np0,
            Np1,
            Np2,
            Np3,
            Np4,
            Np5,
            Np6,
            Np7,
            Np8,
            Np9,
            F1,
            F2,
            F3,
            F4,
            F5,
            F6,
            F7,
            F8,
            F9,
            F10,
            F11,
            F12,
            F13,
            F14,
            F15,
            Pause,

            Last,
        };

        namespace internal {
            // We want the event queue in the window module to actually handle input,
            // because it seems sf::Keyboard::isKeyPressed is very slow compared to keeping
            // track of the input state manually by observing the input events.
            void setState(Key key, bool state);
            void saveLastState();
        }

        bool getKeyboardDown(Key key);
        bool getKeyboardDown(const std::string& keyName);

        bool getKeyboardPressed(Key key);
        bool getKeyboardPressed(const std::string& keyName);

        bool getKeyboardReleased(Key key);
        bool getKeyboardReleased(const std::string& keyName);
    }
}
}
