local input, timer, window = myl.service.input, myl.service.timer, myl.service.window
local tweak = myl.service.tweak
local Tf = tweak.getFloat
local T2 = tweak.getVec2
local Tc = tweak.getColor

myl.loadComponents("components.toml")

tweak.set("playerSpeed", 500.0)

myl.registerSystem(
    "PlayerInput",
    function(dt)
        for entity in myl.foreachEntity(myl.c.PlayerInputState) do
            local pinput = myl.getComponents(entity, myl.c.PlayerInputState)
            local lr = (input.getKeyboardDown("right") and 1 or 0)
                     - (input.getKeyboardDown("left") and 1 or 0)
            local ud = (input.getKeyboardDown("down") and 1 or 0)
                     - (input.getKeyboardDown("up") and 1 or 0)
            pinput.moveDir = myl.vec2(lr, ud):normalized()
        end
    end
)

myl.registerSystem(
    "PlayerMovement",
    function(dt)
        for entity in myl.foreachEntity(myl.c.Transform, myl.c.PlayerInputState) do
            local trafo, pinput = myl.getComponents(entity, myl.c.Transform, myl.c.PlayerInputState)
            trafo.position = trafo.position + pinput.moveDir * Tf("playerSpeed") * dt
        end
    end
)

myl.registerSystem(
    "SetColor",
    function(dt)
        for entity in myl.foreachEntity(myl.c.Color) do
            local col = myl.getComponents(entity, myl.c.Color)
            col.value = Tc("color", myl.color(1, 1, 1, 1))
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
    local circle = myl.addComponent(entity, myl.c.CircleRender)
    circle.radius = 40
    circle.pointCount = 32

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
        myl.invokeSystem("SetColor", dt)

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
