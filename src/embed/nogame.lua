
function love.nogame()

  function love.load()
    love.graphics.setBackgroundColor(0, 138, 225)
  end

  function love.keypressed(key)
    if key == "escape" then
      os.exit()
    end
  end

  local function drawText(str, y)
    local screenw = love.graphics.getWidth()
    local font = love.graphics.getFont()
    love.graphics.print(str, (screenw - font:getWidth(str)) / 2, y)
  end

  function love.draw()
    love.graphics.setColor(255, 255, 255)
    drawText("LoveDOS " .. love.getVersion(), 90)
    drawText("No game", 102)
    love.graphics.setColor(95, 181, 255)
    drawText("Press ESCAPE to quit", 186)
  end

end
