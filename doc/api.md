# LoveDOS API

##### [Modules](#modules-1)
* [love](#love)
* [love.system](#lovesystem)
* [love.graphics](#lovegraphics)
* [love.timer](#lovetimer)
* [love.keyboard](#lovekeyboard)

##### [Objects](#objects-1)
* [Image](#image)
* [Quad](#quad)
* [Font](#font)

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

##### love.graphics.setBackgroundColor([color])
Sets the the background color used when `love.graphics.clear()` is called
without any arguments. If no `color` argument is passed to the function then
the background color is reset to the default.

##### love.graphics.getColor()
Returns the currently set color.

##### love.graphics.setColor([color])
Sets the the color used when drawing. If no `color` argument is passed to the
function then the color is reset to the default.

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

##### love.graphics.getFlip()
Returns true if images are set to be horizontally flipped when drawn

##### love.graphics.setFlip([enable])
Set whether images should be horizontally flipped when draw. If `enable` is not
passed then this is set to false by default.

##### love.graphics.getPalette([index])
Returns 3 numerical values between 0 and 256 representing the rgb color at the
given palette `index`. If no `index` is given then a table with a length of 768
consisting of the value of each channel for each color of the current palette
is returned. This table can be used with `love.graphics.setPalette()`.

##### love.graphics.setPalette(index, r, g, b)
Sets the palette's color at the `index` palette to the rgb color value. `r`,
`g` and `b` should each be a value between 0 and 256.

##### love.graphics.setPalette(t)
Sets the entire palette from the provided table `t`. The table should consist
of 768 numerical values, one for each channel of each rgb color of the palette.
For example, the first number in the table would correspond to the red channel
of the color 0. Each value should be between 0 and 256.

##### love.graphics.reset()
Resets the font, color, background color, canvas, blend mode and flip mode to
their defaults.

##### love.graphics.clear([color])
Clears the screen (or canvas) to the `color`. If no `color` argument is given
then the background color is used.

##### love.graphics.draw(image [, quad], x, y)
Draws the `image` to the screen at the given `x`, `y` position. If a `quad`
argument is provided then the image is clipped to the provided quad when drawn.

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

##### love.graphics.newImage(filename [, key])
Creates and returns a new image. `filename` should be the name of an 8bit .pcx
image file which uses the default VGA palette. The `key` value is the color in
the image which is used to represent the transparent pixels; if no `key` is
provided then the color 0 is used.

##### love.graphics.newCanvas([width, height])
Creates and returns a new blank image of the size `width`, `height`. If a
`width` and `height` are not provided then the image will be the same
dimensions as the screen.

##### love.graphics.newQuad(x, y, width, height)
Creates and returns a new quad.

##### love.graphics.newFont([filename])
Creates and returns a new font. `filename` should be the name of a black and
white 8bit .pcx image file representing all 256 characters in a 16 x 16
character grid. If no `filename` is given then the default embedded font image
is used.

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
##### love.keyboard.isDown(code)
Returns true if the key of the given scancode is currently down.


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
position is outside of the image then 0 is returned.

##### Image:setPixel(x, y, color)
Sets the pixel of the image at the position `x`, `y` to `color`. If the
position is outside of the image then no change is made.

##### Image:mapPixel(fn)
Takes the function `fn` which is called for each pixel of the image and is
given the arguments `x`, `y` and `color`: The position of the current pixel and
its color. Each pixel will be set to the color returned by this function.


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

##### Font:getDimensions(text)
Returns the width and height in pixels which the provided `text` string would
take to render.

##### Font:getWidth([text])
Returns the width in pixels which the provided `text` string would take to
render. If no `text` argument is provided, the width of a single character is
returned.

##### Font:getHeight([text])
Returns the height in pixels which the provided `text` string would take to
render. If no `text` argument is provided, the height of a single line is
returned.

##### Font:setCharSpacing(n)
Sets the size of the gap between each character of text in pixels to `n`.

##### Font:setLineSpacing(n)
Sets the size of the gap between each character of text in pixels to `n`.


## Callbacks
##### love.load()
Called when LoveDOS is started.

##### love.update(dt)
Called at the beginning of each frame, `dt` is the amount of time in seconds
which has passed since the last frame. This is where all the game logic should
take place.

##### love.draw()
Called when the frame is ready to be drawn. All your draw calls should take
place in this function.

##### love.keypressed(code)
Called when the user presses a key. `code` is the scancode value for the
pressed key.

##### love.keyreleased(code)
Called when the user releases a key. `code` is the scancode value for the
released key.

##### love.errhand(err)
Called when an unprotected error occurs in any of the callback functions; `err`
is the error message. Setting this function overrides the default error
behaviour of resetting the VGA mode, printing the error and exiting the
program.

