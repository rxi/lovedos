/**
 * Copyright (c) 2017 Florian Kesseler
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "luaobj.h"
#include "source.h"
#include "mixer.h"


#define CLASS_TYPE LUAOBJ_TYPE_SOURCE
#define CLASS_NAME "Source"


int l_source_new(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);

  source_t *self = luaobj_newudata(L, sizeof(*self));
  luaobj_setclass(L, CLASS_TYPE, CLASS_NAME);
  const char *err = source_init(self, filename);
  if (err) luaL_error(L, err);
  return 1;
}


int l_source_gc(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  source_deinit(self);
  return 0;
}


int l_source_play(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  mixer_play(self);
  return 0;
}


int luaopen_source(lua_State *L) {
  luaL_Reg reg[] = {
    { "new",    l_source_new  },
    { "__gc",   l_source_gc   },
    { "play",   l_source_play },
    { 0, 0 }
  };

  luaobj_newclass(L, CLASS_NAME, NULL, l_source_new, reg);
  return 1;
}
