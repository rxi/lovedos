/** 
 * Copyright (c) 2014 rxi
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
#include "image.h"


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
  keyboard_init();

  /* Init lua */
  L = luaL_newstate();
  lua_atpanic(L, onLuaPanic);
  luaL_openlibs(L);
  luaL_requiref(L, "love", luaopen_love, 1);

  int res = luaL_dostring(L,
    "package.path = package.path .. ';' .."
                   "package.path:gsub('%?', 'lua/?')\n"
    "local print = print\n"
    "local xpcall = xpcall\n"
    "require 'main'\n"
    "if not love.run then\n"
      "function love.run()\n"
        "if love.load then love.load() end\n"
        "love.timer.step()\n"
        "while true do\n"
          /* Keyboard Events*/
          "for _, e in pairs(love.keyboard.getEvents()) do\n"
            "if e.type == 'down' then\n"
              "if love.keypressed then love.keypressed(e.code) end\n"
            "else\n"
              "if love.keyreleased then love.keyreleased(e.code) end\n"
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
    "end\n"
    "if not love.errhand then\n"
      "function love.errhand(msg)\n"
        "love.system.deinitVGA()\n"
        "print('error:')\n"
        "print(msg)\n"
        "print(debug.traceback('', 2))\n"
      "end\n"
    "end\n"
    "xpcall(love.run, love.errhand)\n"
    );

  if (res) {
    vga_deinit();
    const char *err = lua_tostring(L, -1);
    printf("error:\n%s\n", err);
  }

  return 0;
} 
