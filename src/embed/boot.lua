

function love.boot()
  -- Init package.path and add filesystem-compatible package loader
  package.path = "?.lua;?/init.lua"
  table.insert(package.loaders, 1, function(modname)
    modname = modname:gsub("%.", "/")
    for x in package.path:gmatch("[^;]+") do
      local file = x:gsub("?", modname)
      if love.filesystem.exists(file) then
        return assert(loadstring(love.filesystem.read(file), "=" .. file))
      end
    end
  end)

  -- Init event handlers table
  local function makewrapper(name)
    return function(...)
      if love[name] then return love[name](...) end
    end
  end
  love.handlers = {
    ["quit"] = function() end,
    ["mousepressed"] = makewrapper("mousepressed"),
    ["mousereleased"] = makewrapper("mousereleased"),
    ["mousemoved"] = makewrapper("mousemoved"),
    ["keypressed"] = makewrapper("keypressed"),
    ["keyreleased"] = makewrapper("keyreleased"),
    ["textinput"] = makewrapper("textinput"),
   }

  -- Try to mount .exe file, then the first argument
  for i, v in ipairs { love.argv[1], love.argv[2] } do
    local mounted = love.filesystem.mount(v)
    if mounted then
      break
    end
  end

  -- Set write directory and mount
  love.filesystem.setWriteDir("save")
  love.filesystem.mount("save")

  -- Load main.lua or init `nogame` state
  if love.filesystem.isFile("main.lua") then
    require("main")
  else
    love.nogame()
  end

  love.run()
end


function love.run()
  -- Prepare arguments
  local args = {}
  for i = 2, #love.argv do
    args[i - 1] = love.argv[i]
  end

  -- Do load callback
  if love.load then love.load(args) end
  love.timer.step()

  while true do
    -- Handle events
    love.event.pump()
    while 1 do
      local name, a,b,c,d = love.event.poll()
      if not name then
        break
      end
      if name == "quit" then
        if not love.quit or not love.quit() then
          os.exit(a)
        end
      end
      love.handlers[name](a, b, c, d)
    end
    -- Update
    love.timer.step()
    local dt = love.timer.getDelta()
    if love.update then love.update(dt) end
    -- Draw
    love.graphics.clear()
    if love.draw then love.draw() end
    love.graphics.present()
  end
end


function love.errhand(msg)
  -- Init error text
  local err = { "Error\n", msg }
  local trace = debug.traceback("", 2)
  for line in string.gmatch(trace, "([^\t]-)\n") do
    table.insert(err, line)
  end
  local str = table.concat(err, "\n")

  -- Init error state
  love.graphics.reset()
  pcall(love.graphics.setBackgroundColor, 89, 157, 220)

  -- Do error main loop
  while true do
    love.event.pump()
    repeat
      local name, a = love.event.poll()
      if name == "keypressed" and a == "escape" then
        return
      end
    until not name
    love.graphics.clear()
    love.graphics.print(str, 6, 6)
    love.graphics.present()
  end
end


xpcall(love.boot, love.errhand)
