/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "font.h"
#include "luaobj.h"

#define CLASS_TYPE  LUAOBJ_TYPE_FONT
#define CLASS_NAME  "Font"


int l_font_new(lua_State *L) {
  const char *filename;
  int ptsize = 12;
  if ( lua_isnoneornil(L, 2) ) {
    filename = NULL;
    ptsize = luaL_optnumber(L, 1, ptsize);
  } else {
    filename = luaL_checkstring(L, 1);
    ptsize = luaL_optnumber(L, 2, ptsize);
  }
  font_t *self = luaobj_newudata(L, sizeof(*self));
  luaobj_setclass(L, CLASS_TYPE, CLASS_NAME);
  if (filename) {
    const char *err = font_init(self, filename, ptsize);
    if (err) luaL_error(L, err);
  } else {
    font_initEmbedded(self, ptsize);
  }
  return 1;
}


int l_font_gc(lua_State *L) {
  font_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  font_deinit(self);
  return 0;
}


int l_font_getWidth(lua_State *L) {
  font_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  const char *p = luaL_checkstring(L, 2);
  int width = 0;
  while (*p) {
    width += self->glyphs[(int) (*p++ & 127)].xadvance;
  }
  lua_pushinteger(L, width);
  return 1;
}


int l_font_getHeight(lua_State *L) {
  font_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  lua_pushinteger(L, self->height);
  return 1;
}



int luaopen_font(lua_State *L) {
  luaL_Reg reg[] = {
    { "new",            l_font_new            },
    { "__gc",           l_font_gc             },
    { "getWidth",       l_font_getWidth       },
    { "getHeight",      l_font_getHeight      },
    { 0, 0 },
  };
  luaobj_newclass(L, CLASS_NAME, NULL, l_font_new, reg);
  return 1;
}
