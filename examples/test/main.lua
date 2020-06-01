local input, timer, window = myl.service.input, myl.service.timer, myl.service.window
local tweak = myl.service.tweak
local Tf = tweak.getFloat

myl.loadComponents("components.toml")

myl.registerSystem(
    "PlayerMovement",
    function(dt)
        for entity in myl.foreachEntity(myl.c.Transform, myl.c.PlayerInputState) do
            local trafo, pinput = myl.getComponents(entity, myl.c.Transform, myl.c.PlayerInputState)
            trafo.position = trafo.position + pinput.moveDir * Tf("playerSpeed", 500) * dt
        end
    end
)

function myl.main()
    local resX, resY = 1024, 768

    local entity = myl.newEntity()
    myl.addComponent(entity, myl.c.Name).value:set("Player")
    myl.addComponent(entity, myl.c.Color).value = myl.color.new("#af19bf")
    myl.addComponent(entity, myl.c.Transform).position = myl.vec2(resX / 2, resY / 2)
    myl.addComponent(entity, myl.c.PlayerInputState)
    myl.addComponent(entity, myl.c.CircleRender).radius = 40

    entity = myl.newEntity()
    myl.addComponent(entity, myl.c.Name).value:set("Someting")
    myl.addComponent(entity, myl.c.Transform).position = myl.vec2(200, 200)
    myl.addComponent(entity, myl.c.Color).value = myl.color.new("#75e5eb")
    myl.addComponent(entity, myl.c.RectangleRender).size = myl.vec2(120, 120)

    window.init("myl", resX, resY, false)
    --window.setVSync(true)
    local debug = false
    myl.setSystemEnabled("_Debug", debug)
    while window.update() do
        local dt = timer.getDelta()

        if input.getKeyboardDown("lctrl") and input.getKeyboardPressed("d") then
            debug = not debug
            myl.setSystemEnabled("_Debug", debug)
        end

        myl.invokeSystem("PlayerInput", dt)
        myl.invokeSystem("PlayerMovement", dt)

        window.clear()
        myl.invokeSystem("RectangleRender", dt)
        myl.invokeSystem("CircleRender", dt)
        myl.invokeSystem("DrawFps", dt)
        myl.invokeSystem("_Debug", dt)
        window.present()

        if input.getKeyboardDown("escape") then
            break
        end
    end

    tweak.save()
end
