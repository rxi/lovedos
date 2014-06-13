/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <dos.h>
#include <time.h>
#include "lib/dmt/dmt.h"
#include "luaobj.h"
#include "vga.h"


int l_system_getOS(lua_State *L) {
  lua_pushstring(L, "DOS");
  return 1;
}

int l_system_deinitVGA(lua_State *L) {
  vga_deinit();
  return 1;
}


int l_system_getMemUsage(lua_State *L) {
  lua_pushnumber(L, lua_gc(L, LUA_GCCOUNT, 0) + dmt_usage() / 1024);
  return 1;
}



int luaopen_system(lua_State *L) {
  luaL_Reg reg[] = {
    { "getOS",        l_system_getOS        },
    { "getMemUsage",  l_system_getMemUsage  },
    { "deinitVGA",    l_system_deinitVGA    },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
