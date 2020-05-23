local comp = myl.getComponent(0, "MoveComponent")

print(comp.id)
print(comp.flag)

comp.flag = false

myl.registerSystem(
    "HarmonicForceSystem",
    myl.phases.update,
    {"PositionComponent", "VelocityComponent"},
    function(entity, dt)
        local pos = myl.getComponent(entity, "PositionComponent")
        local vel = myl.getComponent(entity, "VelocityComponent")
        vel.value.x = vel.value.x + pos.value.x * 0.5 * dt
        vel.value.y = vel.value.y + pos.value.y * 0.5 * dt
    end
)

myl.registerSystem(
    "MoveSystem",
    myl.phases.postUpdate,
    {"PositionComponent", "VelocityComponent"},
    function(entity, dt)
        local pos = myl.getComponent(entity, "PositionComponent")
        local vel = myl.getComponent(entity, "VelocityComponent")
        pos.value.x = pos.value.x + vel.value.x * dt
        pos.value.y = pos.value.y + vel.value.y * dt
    end
)
