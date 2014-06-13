/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dmt/dmt.h"
#include "font.h"
#include "luaobj.h"


const char *font_init(font_t *self, const char *filename) {
  memset(self, 0, sizeof(*self));
  const char *err = image_init(&self->image, filename, 0x0);
  if (err) return err;
  return NULL;
}


void font_deinit(font_t *self) {
  image_deinit(&self->image);
}


extern int image_blendMode;

void font_blit(font_t *self, pixel_t *buf, int bufw, int bufh,
               const char *str, int dx, int dy
) {
  int cw = self->image.width / 16;
  int ch = self->image.height / 16;
  int cws = (cw + self->charSpacing);
  int chs = (ch + self->lineSpacing);

  const char *p = str;
  int x = dx;
  int y = dy;
  int oldBlendMode = image_blendMode;
  image_blendMode = IMAGE_COLOR;

  while (*p) {
    if (*p == '\n') {
      x = dx;
      y += chs;
    } else {
      if (*p != ' ') { 
        image_blit(&self->image, buf, bufw, bufh,
                   x, y, cw * (*p % 16), ch * (*p / 16), cw, ch);
      }
      x += cws;
    }
    p++;
  }

  image_blendMode = oldBlendMode;
}



#define CLASS_TYPE  LUAOBJ_TYPE_FONT
#define CLASS_NAME  "Font"


int l_font_new(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  font_t *self = luaobj_newudata(L, sizeof(*self));
  luaobj_setclass(L, CLASS_TYPE, CLASS_NAME);
  const char *err = font_init(self, filename);
  if (err) luaL_error(L, err);
  return 1;
}


int l_font_gc(lua_State *L) {
  font_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  font_deinit(self);
  return 0;
}


int l_font_getDimensions(lua_State *L) {
  font_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  int charWidth = (self->image.width / 16) + self->charSpacing;
  int charHeight = (self->image.height / 16) + self->lineSpacing;
  int count = 0, max = 0, lines = 1;
  const char *p = luaL_checkstring(L, 2);
  while (*p) {
    if (*p == '\n') {
      max = count > max ? count : max;
      count = 0;
      lines++;
    } else {
      count++;
    }
    p++;
  }
  max = count > max ? count : max;
  lua_pushinteger(L, max * charWidth);
  lua_pushinteger(L, lines * charHeight);
  return 2;
}


int l_font_getWidth(lua_State *L) {
  font_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  int charWidth = (self->image.width / 16) + self->charSpacing;
  if (lua_isnoneornil(L, 2)) {
    lua_pushinteger(L, charWidth);
    return 1;
  }
  int count = 0, max = 0;
  const char *p = luaL_checkstring(L, 2);
  while (*p) {
    if (*p == '\n') {
      max = count > max ? count : max;
      count = 0;
    } else {
      count++;
    }
    p++;
  }
  max = count > max ? count : max;
  lua_pushinteger(L, max * charWidth);
  return 1;
}


int l_font_getHeight(lua_State *L) {
  font_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  int charHeight = (self->image.height / 16) + self->lineSpacing;
  if (lua_isnoneornil(L, 2)) {
    lua_pushinteger(L, charHeight);
    return 1;
  }
  const char *p = luaL_checkstring(L, 2);
  int lines = 1;
  while (*p) {
    if (*p == '\n') {
      lines++;
    }
    p++;
  }
  lua_pushinteger(L, lines * charHeight);
  return 1;
}


int l_font_setLineSpacing(lua_State *L) {
  font_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  int x = luaL_checkinteger(L, 2);
  self->lineSpacing = x;
  return 0;
}


int l_font_setCharSpacing(lua_State *L) {
  font_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  int x = luaL_checkinteger(L, 2);
  self->charSpacing = x;
  return 0;
}


int luaopen_font(lua_State *L) {
  luaL_Reg reg[] = {
    { "new",            l_font_new            },
    { "__gc",           l_font_gc             },
    { "getDimensions",  l_font_getDimensions  },
    { "getWidth",       l_font_getWidth       },
    { "getHeight",      l_font_getHeight      },
    { "setLineSpacing", l_font_setLineSpacing },
    { "setCharSpacing", l_font_setCharSpacing },
    { 0, 0 },
  };
  luaobj_newclass(L, CLASS_NAME, NULL, l_font_new, reg);
  return 1;
}

