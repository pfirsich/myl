myl.registerSystem(
    "PhysicsIntegrationSystem",
    function(dt)
        print("integrate")
        for entity in myl.foreachEntity(myl.c.Transform, myl.c.Velocity) do
            local trafo, vel = myl.getComponents(entity, myl.c.Transform, myl.c.Velocity)
            trafo.position.x = trafo.position.x + vel.value.x * dt
            trafo.position.y = trafo.position.y + vel.value.y * dt
        end
    end
)

function myl.main()
    print("enter")
    e = myl.newEntity()
    myl.addComponent(e, myl.c.Transform)
    print("added trafo")
    myl.addComponent(e, myl.c.Velocity)

    print("mainloop")

    --myl.service.window.init("Test", 1024, 768)
    while true do
        local dt = 0
        --local dt = myl.service.timer.getDelta()
        myl.invokeSystem("PhysicsIntegrationSystem", dt)
        --myl.service.window.clear()
        --myl.invokeSystem("RectangleRenderSystem", dt)
        --myl.service.window.present()
    end
end
