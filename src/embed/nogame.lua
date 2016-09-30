

function love.nogame()

  function love.load()
    love.graphics.setBackgroundColor(228, 65, 68)
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

  local function drawLines(ycenter, width, height)
    local screenw = love.graphics.getWidth()
    local n = 26
    for i = 0, n - 1 do
      local h = height * math.cos(i * 0.3 + love.timer.getTime() * 3) / 2
      local x = (screenw - width) / 2 + (i / (n - 1)) * width
      love.graphics.line(x, ycenter, x, ycenter + h)
    end
  end

  function love.draw()
    love.graphics.setColor(255, 255, 255)
    drawText("LoveDOS " .. love.getVersion(), 62)
    drawLines(100, 120, 30)
    drawText("No game", 128)
    love.graphics.setColor(243, 150, 152)
    drawText("Press [ESCAPE] to quit", 184)
  end

end
