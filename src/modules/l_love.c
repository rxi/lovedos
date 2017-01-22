/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "luaobj.h"

#define LOVE_VERSION "0.2.1"


int l_love_getVersion(lua_State *L) {
  lua_pushstring(L, LOVE_VERSION);
  return 1;
}


int luaopen_image(lua_State *L);
int luaopen_quad(lua_State *L);
int luaopen_font(lua_State *L);
int luaopen_system(lua_State *L);
int luaopen_event(lua_State *L);
int luaopen_filesystem(lua_State *L);
int luaopen_graphics(lua_State *L);
int luaopen_timer(lua_State *L);
int luaopen_keyboard(lua_State *L);
int luaopen_mouse(lua_State *L);
int luaopen_sound(lua_State *L);
int luaopen_source(lua_State *L);

int luaopen_love(lua_State *L) {
  int i;
  /* Init classes -- all the built in classes are inited here as to create the
   * metatables required by their constructors. Any new classes must also be
   * registered here before they will work. */
  int (*classes[])(lua_State *L) = {
    luaopen_image,
    luaopen_quad,
    luaopen_font,
    luaopen_source,
    NULL,
  };
  for (i = 0; classes[i]; i++) {
    classes[i](L);
    lua_pop(L, 1);
  }

  /* Init love module */
  luaL_Reg reg[] = {
    { "getVersion",   l_love_getVersion },
    { 0, 0 },
  };
  luaL_newlib(L, reg);

  /* Init submodules */
  struct { char *name; int (*fn)(lua_State *L); } mods[] = {
    { "system",     luaopen_system      },
    { "event",      luaopen_event       },
    { "filesystem", luaopen_filesystem  },
    { "graphics",   luaopen_graphics    },
    { "timer",      luaopen_timer       },
    { "keyboard",   luaopen_keyboard    },
    { "mouse",      luaopen_mouse       },
    { "sound",      luaopen_sound       },
    { 0 },
  };
  for (i = 0; mods[i].name; i++) {
    mods[i].fn(L);
    lua_setfield(L, -2, mods[i].name);
  }
  return 1;
}
