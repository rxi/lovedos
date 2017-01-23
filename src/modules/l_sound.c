/**
 * Copyright (c) 2017 Florian Kesseler
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "mixer.h"
#include "luaobj.h"


int l_sound_mix(lua_State *L) {
  mixer_mix();
  return 0;
}


int l_source_new(lua_State *L);


int luaopen_sound(lua_State *L) {
  luaL_Reg reg[] = {
    { "mix",       l_sound_mix },
    { "newSource", l_source_new },
    { 0, 0 }
  };

  luaL_newlib(L, reg);
  return 1;
}
