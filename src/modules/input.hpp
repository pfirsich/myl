#pragma once

#include <string>

namespace myl {
namespace modules {
    namespace input {
        enum class Key { // Keep compatible with sf::Keyboard::Key!
            unknown = -1,
            a = 0,
            b,
            c,
            d,
            e,
            f,
            g,
            h,
            i,
            j,
            k,
            l,
            m,
            n,
            o,
            p,
            q,
            r,
            s,
            t,
            u,
            v,
            w,
            x,
            y,
            z,
            num0,
            num1,
            num2,
            num3,
            num4,
            num5,
            num6,
            num7,
            num8,
            num9,
            escape,
            lctrl,
            lshift,
            lalt,
            lsystem,
            rctrl,
            rshift,
            ralt,
            rsystem,

            menu,
            lbracket,
            rbracket,
            semicolon,
            comma,
            period,
            quote,
            slash,
            backslash,
            tilde,
            equal,
            hyphen,
            space,
            enter,
            backspace,
            tab,
            pageup,
            pagedown,
            end,
            home,
            ins,
            del,
            add,
            subtract,
            multiply,
            divide,
            left,
            right,
            up,
            down,
            np0,
            np1,
            np2,
            np3,
            np4,
            np5,
            np6,
            np7,
            np8,
            np9,
            f1,
            f2,
            f3,
            f4,
            f5,
            f6,
            f7,
            f8,
            f9,
            f10,
            f11,
            f12,
            f13,
            f14,
            f15,
            pause,
            last,
        };

        // Call this at the end of input processing, so the input state can be saved for next frame!
        void update();

        bool getKeyboardDown(Key key);
        bool getKeyboardDown(const std::string& keyName);

        bool getKeyboardPressed(Key key);
        bool getKeyboardPressed(const std::string& keyName);

        bool getKeyboardReleased(Key key);
        bool getKeyboardReleased(const std::string& keyName);
    }
}
}