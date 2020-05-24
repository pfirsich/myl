#pragma once

#include <string>

namespace myl {
namespace modules {
    namespace window {
        void init(const std::string& title, size_t width, size_t height, bool fullscreen);
        void setTitle(const std::string& title);
        bool update();
        void clear();
        void present();
    }
}
}