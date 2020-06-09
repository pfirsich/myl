#include "systems.hpp"

#include "systems/debug.hpp"
#include "systems/drawfps.hpp"
#include "systems/shaperender.hpp"

namespace myl {
void registerBuiltinSystems()
{
    static DrawFpsSystem drawFpsSystem;
    static DebugSystem debugSystem;

    static RectangleRenderSystem rectangleRender;
    static CircleRenderSystem circleRender;
}
}
