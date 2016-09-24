# LoveDOS files

A brief overview of each file and directory which makes up the LoveDOS source
code.

File              | Description
------------------|------------------------------------------------------------
main.c            | The entry point, initialises everything
vga.h             | Function prototypes for initing / deiniting vga mode 13h
vga.c             | Functions for initing / deiniting vga mode 13h
palette.h         | Function prototypes for palette handling
palette.c         | Functions for palette handling
luaobj.h          | Helper function prototypes for lua udata objects
luaobj.c          | Helper functions for lua udata objects
love.c            | The core `love` module
system.c          | `love.system` module
timer.c           | `love.timer` module
graphics.c        | `love.graphics` module
keyboard.h        | `love.keyboard` prototype and typedefs
keyboard.c        | `love.keyboard` module and keyboard interrupt handling
image.h           | `Image` object prototypes and typedefs
image.c           | `Image` object
quad.h            | `Quad` object prototypes and typedefs
quad.c            | `Quad` object
font.h            | `Font` object prototypes and typedefs
font.c            | `Font` object
font\_embedded.c  | The default embedded `Font`'s ttf file
lib/              | Libraries
