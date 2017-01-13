# LoveDOS
A framework for making 2D DOS games in Lua. LoveDOS provides an API based on a
subset of the [LÖVE](https://love2d.org/) API.

![screenshot](https://cloud.githubusercontent.com/assets/3920290/21948750/ed49e9a6-d9e4-11e6-960a-1fac0ec41ee0.gif)

## Getting started
You can download LoveDOS from the
[releases page](https://github.com/rxi/lovedos/releases). If you're not
using DOS as your operating system then [DOSbox](http://www.dosbox.com/) can be
used to emulate a DOS computer.

When you run `love.exe` it will expect its first argument to be your game's
directory. The file `main.lua` will then be searched for and executed. If, for
example, your project was in a directory named `mygame` you would run the
following:
```batch
love mygame
```

A small example program which displays white text on a black background and
exits when the `escape` key is pressed is as follows:

```lua
function love.draw()
  love.graphics.print('Hello World!', 20, 20)
end

function love.keypressed(key)
  if key == "escape" then
    love.event.quit()
  end
end
```

The [doc/api.md](doc/api.md) file provides a reference and overview of all of
the built-in LoveDOS modules, functions and callbacks.

The [doc/packaging.md](doc/packaging.md) file provides instructions for
packaging your game for distribution.


## Building
Instructions for building the project from source can be found in the
[doc/building.md](doc/building.md) file.


## License
This library is free software; you can redistribute it and/or modify it under
the terms of the MIT license. See [LICENSE](LICENSE) for details.

LoveDOS includes Lua (MIT license). The full license for Lua can be found at
the bottom of the [src/lib/lua/lua.h](src/lib/lua/lua.h) file.
