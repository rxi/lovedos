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
#include "filesystem.h"
#include "mouse.h"
#include "image.h"
#include "palette.h"


static lua_State *L;

static void deinit(void) {
  /* Deinit and clear up everything. Called at exit */
  vga_deinit();
  keyboard_deinit();
  lua_close(L);
  filesystem_deinit();
  dmt_dump(stdout);
}


static int onLuaPanic(lua_State *L) {
  vga_deinit();
  const char *err = lua_tostring(L, -1);
  printf("lua panic: %s\n", err);
  return 0;
}


int luaopen_love(lua_State *L);

int main(int argc, char **argv) {

  /* Init everything */
  atexit(deinit);
  filesystem_mount("."); /* Mount cwd: temporary */
  vga_init();
  palette_init();
  keyboard_init();
  mouse_init();

  /* Init lua */
  L = luaL_newstate();
  lua_atpanic(L, onLuaPanic);
  luaL_openlibs(L);
  luaL_requiref(L, "love", luaopen_love, 1);

  /* Create `love.argv` and fill with arguments */
  lua_getglobal(L, "love");
  if (!lua_isnil(L, -1)) {
    lua_newtable(L);
    int i;
    for (i = 0; i < argc; i++) {
      lua_pushstring(L, argv[i]);
      lua_rawseti(L, -2, i + 1);
    }
    lua_setfield(L, -2, "argv");
  }
  lua_pop(L, 1);

  /* Init embedded scripts */
  #include "boot_lua.h"
  int err = luaL_loadbuffer(L, boot_lua, sizeof(boot_lua) - 1, "boot.lua");

  if (err || lua_pcall(L, 0, 0, 0)) {
    vga_deinit();
    const char *err = lua_tostring(L, -1);
    printf("Error\n%s\n", err);
  }

  return 0;
}
