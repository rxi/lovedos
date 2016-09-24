/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dos.h>

#include "lib/dmt/dmt.h"
#include "vga.h"
#include "luaobj.h"
#include "keyboard.h"
#include "mouse.h"
#include "image.h"
#include "palette.h"


static lua_State *L;

static void deinit(void) {
  /* Deinit and clear up everything. Called at exit */
  vga_deinit();
  keyboard_deinit();
  lua_close(L);
  dmt_dump(stdout);
}

static int onLuaPanic(lua_State *L) {
  vga_deinit();
  const char *err = lua_tostring(L, -1);
  printf("lua panic: %s\n", err);
  return 0;
}


int luaopen_love(lua_State *L);

int main(void) {

  /* Init everything */
  atexit(deinit);
  vga_init();
  palette_init();
  keyboard_init();
  mouse_init();

  /* Init lua */
  L = luaL_newstate();
  lua_atpanic(L, onLuaPanic);
  luaL_openlibs(L);
  luaL_requiref(L, "love", luaopen_love, 1);

  const char boot[] =
    "package.path = package.path .. ';' .. package.path:gsub('%?', 'lua/?')\n"
    "local print = print\n"
    "local xpcall = xpcall\n"

    "function love.run()\n"
      "if love.load then love.load() end\n"
      "love.timer.step()\n"
      "while true do\n"
        /* Handle mouse events */
        "for _, e in ipairs(love.mouse.poll()) do\n"
          "if e.type == 'motion' then\n"
            "if love.mousemoved then love.mousemoved(e.x, e.y, e.dx, e.dy) end\n"
          "elseif e.type == 'pressed' then\n"
            "if love.mousepressed then love.mousepressed(e.x, e.y, e.button) end\n"
          "elseif e.type == 'released' then\n"
            "if love.mousereleased then love.mousereleased(e.x, e.y, e.button) end\n"
          "end\n"
        "end\n"
        /* Keyboard events */
        "for _, e in ipairs(love.keyboard.poll()) do\n"
          "if e.type == 'down' then\n"
            "if love.keypressed then love.keypressed(e.key, e.code) end\n"
          "elseif e.type == 'up' then\n"
            "if love.keyreleased then love.keyreleased(e.key, e.code) end\n"
          "elseif e.type == 'text' then\n"
            "if love.textinput then love.textinput(e.text) end\n"
          "end\n"
        "end\n"
        /* Update */
        "love.timer.step()\n"
        "local dt = love.timer.getDelta()\n"
        "if love.update then love.update(dt) end\n"
        /* Draw */
        "love.graphics.clear()\n"
        "if love.draw then love.draw() end\n"
        "love.graphics.present()\n"
      "end\n"
    "end\n"

    "function love.errhand(msg)\n"
      /* Init error text */
      "local err = { 'Error\\n', msg }\n"
      "local trace = debug.traceback('', 2)\n"
      "for line in string.gmatch(trace, '([^\\t]-)\\n') do\n"
        "table.insert(err, line)\n"
      "end\n"
      "local str = table.concat(err, '\\n')"
      /* Init error state */
      "love.graphics.reset()\n"
      "pcall(love.graphics.setBackgroundColor, 89, 157, 220)\n"
      /* Do error main loop */
      "while true do\n"
        "for _, e in ipairs(love.keyboard.poll()) do\n"
          "if e.type == 'down' and e.code == 1 then\n"
            "os.exit()\n"
          "end\n"
        "end\n"
        "love.graphics.clear()\n"
        "love.graphics.print(str, 6, 6)\n"
        "love.graphics.present()\n"
      "end\n"
    "end\n"

    "xpcall(function() require('main') end, love.errhand)\n"
    "xpcall(love.run, love.errhand)\n";

  int err = luaL_loadbuffer(L, boot, sizeof(boot) - 1, "=boot");

  if (err || lua_pcall(L, 0, 0, 0)) {
    vga_deinit();
    const char *err = lua_tostring(L, -1);
    printf("Error\n%s\n", err);
  }

  return 0;
}
