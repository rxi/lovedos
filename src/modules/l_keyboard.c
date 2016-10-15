/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */


#include "keyboard.h"
#include "luaobj.h"


int l_keyboard_setKeyRepeat(lua_State *L) {
  keyboard_setKeyRepeat( lua_toboolean(L, 1) );
  return 0;
}


int l_keyboard_isDown(lua_State *L) {
  int n = lua_gettop(L);
  int res = 0;
  int i;
  for (i = 1; i <= n; i++) {
    const char *key = luaL_checkstring(L, 1);
    res |= keyboard_isDown(key);
  }
  lua_pushboolean(L, res);
  return 1;
}


int luaopen_keyboard(lua_State *L) {
  luaL_Reg reg[] = {
    { "setKeyRepeat", l_keyboard_setKeyRepeat },
    { "isDown",       l_keyboard_isDown       },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
