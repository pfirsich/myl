myl.registerSystem(
    "PhysicsIntegrationSystem",
    function(dt)
        for entity in myl.foreachEntity(myl.c.Transform, myl.c.Velocity) do
            local trafo, vel = myl.getComponents(entity, myl.c.Transform, myl.c.Velocity)
            trafo.position.x = trafo.position.x + vel.value.x * dt
            trafo.position.y = trafo.position.y + vel.value.y * dt
        end
    end
)

function myl.main()
    print("enter")
    local entity = myl.newEntity()
    local trafo = myl.addComponent(entity, myl.c.Transform)
    trafo.position.x = 512
    trafo.position.y = 384
    myl.addComponent(entity, myl.c.Velocity)
    local rect = myl.addComponent(entity, myl.c.RectangleRender)
    rect.size.x = 50
    rect.size.y = 50

    print("mainloop")
    myl.service.window.init("myl", 1024, 768, false)
    while myl.service.window.update() do
        local dt = myl.service.timer.getDelta()
        myl.invokeSystem("PhysicsIntegrationSystem", dt)
        myl.service.window.clear()
        myl.invokeSystem("RectangleRenderSystem", dt)
        myl.service.window.present()
    end
end
