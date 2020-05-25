local playerSpeed = 500

myl.registerSystem(
    "PlayerMovementSystem",
    function(dt)
        for entity in myl.foreachEntity(myl.c.Transform, myl.c.PlayerInputState) do
            local trafo, pinput = myl.getComponents(entity, myl.c.Transform, myl.c.PlayerInputState)
            trafo.position = trafo.position + pinput.moveDir * playerSpeed * dt
        end
    end
)

function myl.main()
    local entity = myl.newEntity()
    local trafo = myl.addComponent(entity, myl.c.Transform)
    trafo.position = myl.vec2(512, 384)
    myl.addComponent(entity, myl.c.PlayerInputState)
    local rect = myl.addComponent(entity, myl.c.RectangleRender)
    rect.size = myl.vec2(50, 50)

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
