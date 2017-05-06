/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

 #include "lib/cmixer/cmixer.h"
 #include "luaobj.h"


int l_audio_setVolume(lua_State *L) {
  double n = luaL_checknumber(L, 1);
  cm_set_master_gain(n);
  return 0;
}


int l_source_new(lua_State *L);

int luaopen_audio(lua_State *L) {
  luaL_Reg reg[] = {
    { "newSource",  l_source_new        },
    { "setVolume",  l_audio_setVolume   },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
