/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "luaobj.h"
#include "palette.h"
#include "image.h"


#define CLASS_TYPE  LUAOBJ_TYPE_IMAGE
#define CLASS_NAME  "Image"


int l_image_new(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  image_t *self = luaobj_newudata(L, sizeof(*self));
  luaobj_setclass(L, CLASS_TYPE, CLASS_NAME);
  const char *err = image_init(self, filename);
  if (err) luaL_error(L, err);
  return 1;
}


int l_image_newCanvas(lua_State *L) {
  int width = VGA_WIDTH;
  int height = VGA_HEIGHT;
  if (lua_gettop(L) > 0) {
    width = luaL_checknumber(L, 1);
    height = luaL_checknumber(L, 2);
    if (width <= 0) luaL_argerror(L, 1, "width must be larger than 0");
    if (height <= 0) luaL_argerror(L, 2, "height must be larger than 0");
  }
  image_t *self = luaobj_newudata(L, sizeof(*self));
  luaobj_setclass(L, CLASS_TYPE, CLASS_NAME);
  image_initBlank(self, width, height);
  return 1;
}


int l_image_gc(lua_State *L) {
  image_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  image_deinit(self);
  return 0;
}


int l_image_getDimensions(lua_State *L) {
  image_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  lua_pushinteger(L, self->width);
  lua_pushinteger(L, self->height);
  return 2;
}


int l_image_getWidth(lua_State *L) {
  image_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  lua_pushinteger(L, self->width);
  return 1;
}


int l_image_getHeight(lua_State *L) {
  image_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  lua_pushinteger(L, self->height);
  return 1;
}


int l_image_getPixel(lua_State *L) {
  image_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  int x = luaL_checknumber(L, 2);
  int y = luaL_checknumber(L, 3);
  if (x < 0 || x >= self->width || y < 0 || y >= self->height) {
    /* Return `nil` for out of bounds (same as transparent) */
    lua_pushnil(L);
    return 1;
  } else {
    /* Return `nil` if color is transparent, else return 3 channel values */
    int idx = self->data[x + y * self->width];
    if (idx == 0) {
      lua_pushnil(L);
      return 1;
    } else {
      int rgb[3];
      palette_idxToColor(idx, rgb);
      lua_pushinteger(L, rgb[0]);
      lua_pushinteger(L, rgb[1]);
      lua_pushinteger(L, rgb[2]);
      return 3;
    }
  }
}


int l_image_setPixel(lua_State *L) {
  image_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  int x = luaL_checknumber(L, 2);
  int y = luaL_checknumber(L, 3);
  if (lua_isnoneornil(L, 4)) {
    /* Set transparent */
    image_setPixel(self, x, y, 0);
    image_setMaskPixel(self, x, y, 0xff);
  } else {
    /* Get color, set pixel and mask */
    int r = luaL_checknumber(L, 4);
    int g = luaL_checknumber(L, 5);
    int b = luaL_checknumber(L, 6);
    int idx = palette_colorToIdx(r, g, b);
    if (idx < -1) {
      luaL_error(L, "color palette exhausted: use fewer unique colors");
    }
    image_setPixel(self, x, y, idx);
    image_setMaskPixel(self, x, y, 0x0);
  }
  return 0;
}


int luaopen_image(lua_State *L) {
  luaL_Reg reg[] = {
    { "new",            l_image_new           },
    { "__gc",           l_image_gc            },
    { "getDimensions",  l_image_getDimensions },
    { "getWidth",       l_image_getWidth      },
    { "getHeight",      l_image_getHeight     },
    { "getPixel",       l_image_getPixel      },
    { "setPixel",       l_image_setPixel      },
    { 0, 0 },
  };
  luaobj_newclass(L, CLASS_NAME, NULL, l_image_new, reg);
  return 1;
}
