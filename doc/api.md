# LoveDOS API

##### [Modules](#modules-1)
* [love](#love)
* [love.system](#lovesystem)
* [love.graphics](#lovegraphics)
* [love.timer](#lovetimer)
* [love.keyboard](#lovekeyboard)
* [love.mouse](#lovemouse)
* [love.filesystem](#lovefilesystem)
* [love.audio](#loveaudio)
* [love.event](#loveevent)

##### [Objects](#objects-1)
* [Image](#image)
* [Quad](#quad)
* [Font](#font)
* [Source](#source)

##### [Callbacks](#callbacks-1)


## Modules

### love

##### love.getVersion()
Returns the version of LoveDOS as a string.

### love.system
Provides access to information about the user's system.

##### love.system.getOS()
Returns the operating system which LoveDOS is running on.

##### love.system.getMemUsage()
Returns the amount of memory in kilobytes which is being used by LoveDOS. This
includes the memory used by both the loaded assets and lua.


### love.graphics
Provides functions for drawing lines, shapes, text and images.

##### love.graphics.getDimensions()
Returns the width and height of the screen in pixels as two numbers.

##### love.graphics.getWidth()
Returns the width of the screen in pixels.

##### love.graphics.getHeight()
Returns the height of the screen in pixels.

##### love.graphics.getBackgroundColor()
Returns the currently set background color.

##### love.graphics.setBackgroundColor(red, green, blue)
Sets the the background color used when `love.graphics.clear()` is called
without any arguments. If called with no arguments the back color is set to
black.

##### love.graphics.getColor()
Returns the currently set color.

##### love.graphics.setColor(red, green, blue)
Sets the the color used when drawing. If called with no arguments the color is
set to white.

##### love.graphics.getBlendMode()
Returns the currently set blend mode.

##### love.graphics.setBlendMode([mode])
Sets the current blend mode used by `love.graphics.draw()`. If no `mode`
argument is passed then the blend mode is set to the default (`"normal"`).

Mode        | Description
------------|------------------------------------------------------------------
`"normal"`  | Draws normally with transparency
`"fast"`    | Draws without transparency, this is the fastest blend mode
`"and"`     | Binary ANDs the source and destination pixels
`"or"`      | Binary ORs the source and destination pixels
`"color"`   | Draws opaque pixels using the `love.graphics.setColor()` color

##### love.graphics.getFont()
Returns the current font.

##### love.graphics.setFont([font])
Sets the current font. If `font` is nil then the font is reset to the default.

##### love.graphics.getCanvas()
Returns the current canvas.

##### love.graphics.setCanvas([image])
Sets the current canvas which all the draw operations will draw to. If the
`image` argument is not set then the canvas is reset to the default canvas
representing the user's screen.

##### love.graphics.reset()
Resets the font, color, background color, canvas, blend mode and flip mode to
their defaults.

##### love.graphics.clear(red, green, blue)
Clears the screen (or canvas) to the color. If no color argument is given
then the background color is used (see `love.graphics.setBackgroundColor()`).

##### love.graphics.draw(image [, quad] [, x [, y [, flip]]])
Draws the `image` to the screen at the given `x`, `y` position. If a `quad`
argument is provided then the image is clipped to the provided quad when drawn.
If `flip` is true then the image is flipped horizontally.

##### love.graphics.point(x, y)
Draws a pixel.

##### love.graphics.line(x, y, x2, y2 [, ...])
Draws a line from the positition `x`, `y` to `x2`, `y2`. You can continue
passing point positions to draw a polyline.

##### love.graphics.rectangle(mode, x, y, width, height)
Draws a rectange and the `x`, `y` position of the given `width` and `height`.
`mode` should be either `"fill"` or `"line"`.

##### love.graphics.circle(mode, x, y, radius)
Draws a circle of a given `radius` with its center at the `x`, `y` position.
`mode` should be either `"fill"` or `"line"`.

##### love.graphics.print(text, x, y)
Draws the `text` string in the current font with its top left at the `x`, `y`
position.

##### love.graphics.newImage(filename)
Creates and returns a new image. `filename` should be the name of an image file.
LoveDOS is limited to a palette of 255 unique colors in any given game; it is up
to the user not to exceed this limit.

##### love.graphics.newCanvas([width, height])
Creates and returns a new blank image of the size `width`, `height`. If a
`width` and `height` are not provided then the image will be the same
dimensions as the screen.

##### love.graphics.newQuad(x, y, width, height)
Creates and returns a new quad.

##### love.graphics.newFont([filename ,] ptsize)
Creates and returns a new font. `filename` should be the name of a ttf file and
`ptsize` its size. If no `filename` is provided the built in font is used.

##### love.graphics.present()
Flips the current screen buffer with the displayed screen buffer. This is
called automatically after the `love.draw()` callback.


### love.timer
Provides an interface to your system's clock.

##### love.timer.getDelta()
Returns the time between the last two frames.

##### love.timer.getAverageDelta()
Returns the average delta time over the last second.

##### love.timer.getFps()
Returns the current frames per second

##### love.timer.getTime()
Returns the number of seconds since some time in the past. The value returned
by this function should be used only as a comparison with other values returned
by this function.

##### love.timer.step()
Measures the time between two frames. This is automatically called each frame.

##### love.timer.sleep(seconds)
Pauses the thread for the specified number of `seconds`. During this time no
callbacks are called.


### love.keyboard
##### love.keyboard.isDown(key, ...)
Returns true if any of the given keys are currently pressed.

##### love.keyboard.setKeyRepeat(enable)
Dictates whether repeat `keypressed` events should occur if a key is held down.
By default this is `false`.


### love.mouse
##### love.mouse.getPosition()
Returns 2 values: the current horizontal and vertical position of the mouse.

##### love.mouse.getX()
Returns the horizontal position of the mouse.

##### love.mouse.getY()
Returns the vertical position of the mouse.

##### love.mouse.isDown(button, ...)
Returns true if any of the given mouse buttons are currently pressed.  `button`
should be the value `1` (left), `2` (right) or `3` (middle).


### love.filesystem
##### love.filesystem.mount(path)
Mounts the given path. `path` should be either a directory or tar archive.
Returns `nil` and an error message on failure.

##### love.filesystem.unmount(path)
Unmounts the given `path`. Returns `nil` and an error message on failure.

##### love.filesystem.exists(path)
Returns `true` if the given path exists.

##### love.filesystem.isFile(filename)     
Returns `true` if `filename` is a file.

##### love.filesystem.isDirectory(dir)
Returns `true` if `dir` is a directory.

##### love.filesystem.read(filename)
Reads and returns the contents of the file at `filename`.

##### love.filesystem.write(filename, string)
Writes `string` to the given `filename` in the game's save directory.


### love.audio
##### love.audio.newSource(filename)
Creates and returns a new audio source. `filename` should the filename of the
`.wav` file to load.

##### love.audio.setVolume(volume)
Sets the master volume, by default this is `1`.


### love.event
##### love.event.quit([status])
Pushes the `quit` event with the given `status`. `status` is `0` by default.


## Objects
### Image
A loaded image or canvas which can be drawn.

##### Image:getDimensions()
Returns the width and height of the image in pixels as two numbers.

##### Image:getWidth()
Returns the width in pixels of the image.

##### Image:getHeight()
Returns the height in pixels of the image.

##### Image:getPixel(x, y)
Returns the color of the pixel at the position `x`, `y` of the image. If the
position is out of bounds or the pixel is set to transparent then `nil` is
returned.

##### Image:setPixel(x, y [, red, green, blue])
Sets the pixel of the image at position `x`, `y` to the given color; if no color
is provided the pixel is set to transparent. If the position is out of bounds
then no change is made.


### Quad
A rectangle used to represent the clipping region of an image when drawing.

##### Quad:setViewport(x, y, width, height)
Sets the position and dimensions of the quad

##### Quad:getViewport()
Returns the position (`x`, `y`) and dimensions (`width`, `height`) of the quad,
4 numerical values in total.


### Font
A font used by `love.graphics.print()`. The current font can be set using the
`love.graphics.setFont()` function.

##### Font:getWidth(text)
Returns the width in pixels that the `text` string would take when printed using
this font.

##### Font:getHeight()
Returns the height of the font in pixels.


### Source
##### Source:setVolume(volume)
Sets the volume -- by default this is `1`.

##### Source:setPitch(pitch)
Sets the pitch (playback speed). By default this is `1`. `0.5` is half the
pitch, `2` is double the pitch.

##### Source:setLooping(enable)
Enables looping if `enable` is `true`. By default looping is disabled.

##### Source:getDuration()
Gets the length in seconds of the source's audio data.

##### Source:isPlaying()
Returns `true` if the source is currently playing.

##### Source:isPaused()
Returns `true` if the source is currently paused.

##### Source:isStopped()
Returns `true` if the source is currently stopped.

##### Source:tell()
Returns the current playback position in seconds.

##### Source:play()
Plays the audio source. If the source is already playing then this function has
no effect. To play back from the start call `Source:stop()` before calling this
function.

##### Source:pause()
Pauses the source's playback. This stops playback without losing the current position, calling `Source:play()` will continue playing where it left off.

##### Source:stop()
Stops playing and rewinds the source's play position back to the beginning.


## Callbacks
##### love.load(args)
Called when LoveDOS is started. `args` is a table containing the command line
arguments passed to LoveDOS.

##### love.update(dt)
Called at the beginning of each frame, `dt` is the amount of time in seconds
which has passed since the last frame. This is where all the game logic should
take place.

##### love.draw()
Called when the frame is ready to be drawn. All your draw calls should take
place in this function.

##### love.keypressed(key, code, isrepeat)
Called when the user presses a key. `key` is the key that was pressed, `code` is
the scancode of the pressed key, `isrepeat` is true if this key press event is a
repeat.

##### love.keyreleased(key, code)
Called when the user releases a key. `key` is the key that was released, `code`
is the scancode for the released key.

##### love.textinput(text)
Called when text has been entered by the user. For example if shift-2 is pressed
on an American keyboard layout, the text "@" will be generated.

##### love.mousemoved(x, y, dx, dy)
Called when the user moves the mouse. `x` and `y` are the mouse's current
position, `dx` and `dy` is the amount moved relative to the last mouse position.

##### love.mousepressed(x, y, button)
Called when the user presses a mouse button. `x` and `y` are the mouse's current
position. `button` is the value `1` (left), `2` (right) or `3` (middle).

##### love.mousereleased(x, y, button)
Called when the user releases a mouse button. `x` and `y` are the mouse's
current position. `button` is the value `1` (left), `2` (right) or `3` (middle).

##### love.errhand(err)
Called when an unprotected error occurs; `err` is the error message. By default
this function displays the error message and stacktrace on screen and waits for
the `escape` key to be pressed before exiting.
