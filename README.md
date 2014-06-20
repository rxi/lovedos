# LoveDOS
A framework for making 2D DOS games in Lua. LoveDOS provides a subset of the
[LÖVE](https://love2d.org/) API.

![screenshot](https://cloud.githubusercontent.com/assets/3920290/3274842/db280102-f334-11e3-9967-f27f01d34d52.gif)

## Getting started
You can download LoveDOS from the
[releases page](https://github.com/rxi/lovedos/releases). If you're not
using DOS as your operating system then [DOSbox](http://www.dosbox.com/) can be
used to emulate a DOS computer.

When you run love.exe it searches for a file named "main.lua" in its current
folder; the code in "main.lua" is then loaded and executed. A small example
program which displays white text on a black background and exits when the
`escape` key is pressed is as follows:

```lua
function love.draw()
  love.graphics.print('Hello World!', 20, 20)
end

function love.keypressed(code)
  if code == 1 then os.exit() end
end
```

The [doc/api.md](doc/api.md) file provides a reference and overview of all of
the built-in LoveDOS modules, functions and callbacks.


## Using images
LoveDOS provides support for a single image format: 8bit .pcx; the palette
stored in the image is ignored. LoveDOS uses the default VGA palette by
default, but this can be changed by using the `love.graphics.setPalette()`
function. [ASEprite](http://www.aseprite.org/) is a good choice of image
editing software for use with LoveDOS as it uses the default VGA palette by
default and can save to the 8bit .pcx format.


## Building
Instructions for building the project from source can be found in the
[doc/building.md](doc/building.md) file. If you intend to make changes to the
project then an overview of each source file's content can be found in
[doc/files.md](doc/files.md).


## License
This library is free software; you can redistribute it and/or modify it under
the terms of the MIT license. See [LICENSE](LICENSE) for details.

LoveDOS includes Lua (MIT license). The full license for Lua can be found at
the bottom of the [src/lib/lua/lua.h](src/lib/lua/lua.h) file.
