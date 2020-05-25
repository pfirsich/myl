local playerSpeed = 500

myl.registerSystem(
    "PlayerMovement",
    function(dt)
        for entity in myl.foreachEntity(myl.c.Transform, myl.c.PlayerInputState) do
            local trafo, pinput = myl.getComponents(entity, myl.c.Transform, myl.c.PlayerInputState)
            trafo.position = trafo.position + pinput.moveDir * playerSpeed * dt
        end
    end
)

function myl.main()
    local resX, resY = 1024, 768

    local entity = myl.newEntity()
    local trafo = myl.addComponent(entity, myl.c.Transform)
    trafo.position = myl.vec2(resX/2, resY/2)
    myl.addComponent(entity, myl.c.PlayerInputState)
    local rect = myl.addComponent(entity, myl.c.RectangleRender)
    rect.size = myl.vec2(50, 50)

    myl.service.window.init("myl", resX, resY, false)
    while myl.service.window.update() do
        local dt = myl.service.timer.getDelta()

        myl.invokeSystem("PlayerInput", dt)
        myl.invokeSystem("PlayerMovement", dt)

        myl.service.window.clear()
        myl.invokeSystem("RectangleRender", dt)
        myl.invokeSystem("DrawFps", dt)
        myl.service.window.present()

        if myl.service.input.getKeyboardDown("escape") then
            break
        end
    end
end
