local playerSpeed = 500

myl.registerSystem(
    "PlayerMovementSystem",
    function(dt)
        for entity in myl.foreachEntity(myl.c.Transform, myl.c.PlayerInputState) do
            local trafo, pinput = myl.getComponents(entity, myl.c.Transform, myl.c.PlayerInputState)
            trafo.position.x = trafo.position.x + pinput.moveDir.x * playerSpeed * dt
            trafo.position.y = trafo.position.y + pinput.moveDir.y * playerSpeed * dt
        end
    end
)

function myl.main()
    print("enter")
    local entity = myl.newEntity()
    local trafo = myl.addComponent(entity, myl.c.Transform)
    trafo.position.x = 512
    trafo.position.y = 384
    myl.addComponent(entity, myl.c.PlayerInputState)
    local rect = myl.addComponent(entity, myl.c.RectangleRender)
    rect.size.x = 50
    rect.size.y = 50

    print("mainloop")
    myl.service.window.init("myl", 1024, 768, false)
    while myl.service.window.update() do
        local dt = myl.service.timer.getDelta()
        myl.invokeSystem("PlayerInputSystem", dt)
        myl.invokeSystem("PlayerMovementSystem", dt)
        myl.service.window.clear()
        myl.invokeSystem("RectangleRenderSystem", dt)
        myl.invokeSystem("DrawFpsSystem", dt)
        myl.service.window.present()

        if myl.service.input.getKeyboardDown("escape") then
            break
        end
    end
end
