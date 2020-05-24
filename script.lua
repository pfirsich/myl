myl.registerSystem(
    "HarmonicForceSystem",
    {"PositionComponent", "VelocityComponent"},
    function(entity, dt)
        print("harmonic")
        local pos = myl.getComponent(entity, "PositionComponent")
        local vel = myl.getComponent(entity, "VelocityComponent")
        vel.value.x = vel.value.x + pos.value.x * 0.5 * dt
        vel.value.y = vel.value.y + pos.value.y * 0.5 * dt
    end
)

myl.registerSystem(
    "MoveSystem",
    {"PositionComponent", "VelocityComponent"},
    function(entity, dt)
        print("move")
        local pos = myl.getComponent(entity, "PositionComponent")
        local vel = myl.getComponent(entity, "VelocityComponent")
        pos.value.x = pos.value.x + vel.value.x * dt
        pos.value.y = pos.value.y + vel.value.y * dt
    end
)

function myl.main()
    print("enter")
    e = myl.newEntity()
    print("entity: ", e)
    print("add harm")
    myl.addComponent(e, "PositionComponent")
    print("add move")
    myl.addComponent(e, "VelocityComponent")

    print("mainloop")

    --myl.service.window.init("Test", 1024, 768)
    while true do
        local dt = 0
        --local dt = myl.service.timer.getDelta()
        myl.invokeSystem("HarmonicForceSystem", dt)
        myl.invokeSystem("MoveSystem", dt)
    end
end
