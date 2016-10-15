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


int l_keyboard_poll(lua_State *L) {
  lua_newtable(L);
  keyboard_Event e;
  int idx = 1;

  while (keyboard_poll(&e)) {

    if (e.type == KEYBOARD_PRESSED || e.type == KEYBOARD_RELEASED) {
      lua_newtable(L);
      lua_pushstring(L, e.type == KEYBOARD_PRESSED ? "down" : "up");
      lua_setfield(L, -2, "type");
      lua_pushnumber(L, e.code);
      lua_setfield(L, -2, "code");
      lua_pushstring(L, e.key);
      lua_setfield(L, -2, "key");
      if (e.type == KEYBOARD_PRESSED) {
        lua_pushboolean(L, e.isrepeat);
        lua_setfield(L, -2, "isrepeat");
      }
      lua_rawseti(L, -2, idx++);

    } else if (e.type == KEYBOARD_TEXTINPUT) {
      lua_newtable(L);
      lua_pushstring(L, "text");
      lua_setfield(L, -2, "type");
      lua_pushstring(L, e.text);
      lua_setfield(L, -2, "text");
      lua_rawseti(L, -2, idx++);
    }
  }

  return 1;
}


int luaopen_keyboard(lua_State *L) {
  luaL_Reg reg[] = {
    { "poll",         l_keyboard_poll         },
    { "setKeyRepeat", l_keyboard_setKeyRepeat },
    { "isDown",       l_keyboard_isDown       },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
