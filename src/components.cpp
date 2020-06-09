#include "components.hpp"

#include "ecs.hpp"
#include "systems/shaperender.hpp"

namespace c = myl::components;

namespace myl {
void registerBuiltinComponents()
{
    myl::registerComponent("Name", myl::StructBuilder().addField("value", &c::Name::value).build());

    myl::registerComponent("Transform",
        myl::StructBuilder()
            .addField("position", &c::Transform::position)
            .addField("angle", &c::Transform::angle)
            .addField("scale", &c::Transform::scale)
            .addField("origin", &c::Transform::origin)
            .build());

    myl::registerComponent(
        "Color", myl::StructBuilder().addField("value", &c::Color::value).build());

    myl::registerComponent("RectangleRender",
        myl::StructBuilder().addField("size", &c::RectangleRender::size).build());

    myl::registerComponent("CircleRender",
        myl::StructBuilder()
            .addField("radius", &c::CircleRender::radius)
            .addField("pointCount", &c::CircleRender::pointCount)
            .build());
}
}
