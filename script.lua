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
    myl.addComponent(entity, myl.c.Transform).position = myl.vec2(resX/2, resY/2)
    myl.addComponent(entity, myl.c.PlayerInputState)
    myl.addComponent(entity, myl.c.RectangleRender).size = myl.vec2(50, 50)

    entity = myl.newEntity()
    myl.addComponent(entity, myl.c.Transform).position = myl.vec2(200, 200)
    myl.addComponent(entity, myl.c.PlayerInputState)
    myl.addComponent(entity, myl.c.RectangleRender).size = myl.vec2(50, 50)

    myl.service.window.init("myl", resX, resY, false)
    local debug = false
    while myl.service.window.update() do
        local dt = myl.service.timer.getDelta()

        if myl.service.input.getKeyboardDown("lctrl") and
                myl.service.input.getKeyboardPressed("d") then
            debug = not debug
        end

        myl.invokeSystem("PlayerInput", dt)
        myl.invokeSystem("PlayerMovement", dt)

        myl.service.window.clear()
        myl.invokeSystem("RectangleRender", dt)
        myl.invokeSystem("DrawFps", dt)
        if debug then
            myl.invokeSystem("EntityInspector", dt)
        end
        myl.service.window.present()

        if myl.service.input.getKeyboardDown("escape") then
            break
        end
    end
end
