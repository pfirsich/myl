local playerSpeed = 500

myl.registerSystem(
    "PlayerMovementSystem",
    function(dt)
        print("movement")
        for entity in myl.foreachEntity(myl.c.Transform, myl.c.PlayerInputState) do
            print(entity)
            local trafo, input = myl.getComponents(myl.cTransform, myl.c.PlayerInputState)
            trafo.position.x = trafo.position.x + input.moveDir.x * playerSpeed * dt
            trafo.position.y = trafo.position.y + input.moveDir.y * playerSpeed * dt
            print("pos", trafo.position.x, trafo.position.y)
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
        myl.service.window.present()

        if myl.service.input.getKeyboardDown("escape") then
            break
        end
    end
end
