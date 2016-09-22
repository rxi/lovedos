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
#include "lib/stb/stb_image.h"
#include "image.h"
#include "palette.h"
#include "luaobj.h"

int image_blendMode = IMAGE_NORMAL;
int image_flip = 0;
unsigned int image_color = 0x0f0f0f0f;


void image_setBlendMode(int mode) {
  image_blendMode = mode;
}

void image_setColor(pixel_t color) {
  image_color = color | (color << 8) | (color << 16) | (color << 24);
}

void image_setFlip(int mode) {
  image_flip = !!mode;
}



const char *image_init(image_t *self, const char *filename) {
  /* Loads an image file into the struct and inits the mask */
  const char *errmsg = NULL;
  memset(self, 0, sizeof(*self));

  /* Load 32bit image data */
  int width, height, n;
  unsigned char *data32 = stbi_load(filename, &width, &height, &n, 4);
  if (!data32) {
    errmsg = "could not load image file";
    goto fail;
  }

  /* Set dimensions and allocate memory */
  int sz = width * height;
  self->width = width;
  self->height = height;
  self->data = dmt_malloc(sz);

  /* Load pixels into struct, converting 32bit to 8bit paletted */
  int i;
  for (i = 0; i < width * height; i++) {
    unsigned char *p = data32 + i * 4;
    int b = p[0];
    int g = p[1];
    int r = p[2];
    int a = p[3];
    int idx = palette_colorIdx(r, g, b);
    if (idx < 0) {
      errmsg = "color palette exhausted: use fewer unique colors";
      goto fail;
    }
    self->data[i] = (a >= 127) ? idx : 0;
  }

  /* Init mask */
  self->mask = dmt_malloc(sz);
  for (i = 0; i < sz; i++) {
    self->mask[i] = (self->data[i] == 0) ? 0xFF : 0x00;
  }

  /* Free 32bit pixel data, return NULL for no error */
  free(data32);
  data32 = NULL;

  return NULL;

fail:
  free(data32);
  return errmsg;
}


void image_initBlank(image_t *self, int width, int height) {
  /* Creates a blank zeroset image with a zeroset mask. This function can be
   * used to init the image instead of image_init() */
  memset(self, 0, sizeof(*self));
  self->data = dmt_calloc(1, width * height);
  self->width = width;
  self->height = height;
  /* Init mask */
  self->mask = dmt_calloc(1, width * height);
}


void image_blit(image_t *self, pixel_t *buf, int bufw, int bufh,
                int dx, int dy, int sx, int sy, int sw, int sh
) {
  int diff;

  /* Clip to source buffer */
  if (sx < 0) { sw -= sx; sx = 0; }
  if (sy < 0) { sy -= sy; sy = 0; }
  if ((diff = (sx + sw) - self->width) > 0) { sw -= diff; }
  if ((diff = (sy + sh) - self->height) > 0) { sh -= diff; }

  /* Clip to destination buffer */
  if (!image_flip) {
    if ((diff = -dx) > 0) { sw -= diff; sx += diff; dx += diff; }
    if ((diff = dx + sw - bufw) >= 0) { sw -= diff; }
  } else {
    if ((diff = -dx) > 0) { sw -= diff; dx += diff; }
    if ((diff = dx + sw - bufw) >= 0) { sx += diff; sw -= diff; }
  }
  if ((diff = dy + sh - bufh) >= 0) { sh -= diff; }
  if ((diff = -dy) > 0) { sh -= diff; sy += diff; dy += diff; }

  /* Return early if we're clipped entirely off the dest / source */
  if (sw <= 0 || sh <= 0) return;

  /* Blit */
  #define BLIT_LOOP_NORMAL(func)\
    {\
      int x, y;\
      int srci = sx + sy * self->width;\
      int dsti = dx + dy * bufw;\
      int srcrowdiff = self->width - sw;\
      int dstrowdiff = bufw - sw;\
      int sw32 = sw - (sw & 3);\
      for (y = 0; y < sh; y++) {\
        for (x = 0; x < sw32; x += 4) {\
          func(*(unsigned int*)&buf[dsti],\
               *(unsigned int*)&self->data[srci],\
               *(unsigned int*)&self->mask[srci])\
          srci += 4;\
          dsti += 4;\
        }\
        for (; x < sw; x++) {\
          func(buf[dsti], self->data[srci], self->mask[srci])\
          srci++;\
          dsti++;\
        }\
        srci += srcrowdiff;\
        dsti += dstrowdiff;\
      }\
    }

  #define BLIT_LOOP_FLIPPED(func)\
    {\
      int x, y;\
      int srci = sx + sy * self->width + sw - 1;\
      int dsti = dx + dy * bufw;\
      int srcrowdiff = self->width + sw;\
      int dstrowdiff = bufw - sw;\
      for (y = 0; y < sh; y++) {\
        for (x = 0; x < sw; x++) {\
          func(buf[dsti], self->data[srci], self->mask[srci])\
          srci--;\
          dsti++;\
        }\
        srci += srcrowdiff;\
        dsti += dstrowdiff;\
      }\
    }

  #define BLIT_NORMAL(dst, src, msk)\
    (dst) &= (msk);\
    (dst) |= (src);

  #define BLIT_AND(dst, src, msk)\
    (dst) &= (src);

  #define BLIT_OR(dst, src, msk)\
    (dst) |= (src);

  #define BLIT_COLOR(dst, src, msk)\
    (dst) &= (msk);\
    (dst) |= ~(msk) & image_color;

  #define BLIT(blit_loop)\
    switch (image_blendMode) {\
      default:\
      case IMAGE_NORMAL : blit_loop(BLIT_NORMAL)  break;\
      case IMAGE_AND    : blit_loop(BLIT_AND)     break;\
      case IMAGE_OR     : blit_loop(BLIT_OR)      break;\
      case IMAGE_COLOR  : blit_loop(BLIT_COLOR)   break;\
    }\

  if (!image_flip) {
    if (image_blendMode == IMAGE_FAST) {
      int y;
      int srci = sx + sy * self->width;
      int dsti = dx + dy * bufw;
      for (y = 0; y < sh; y++) {
        memcpy(buf + dsti, self->data + srci, sw);
        srci += self->width;
        dsti += bufw;
      }
    } else {
      BLIT(BLIT_LOOP_NORMAL);
    }
  } else {
    BLIT(BLIT_LOOP_FLIPPED);
  }

}


void image_deinit(image_t *self) {
  dmt_free(self->data);
  dmt_free(self->mask);
}



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
    width = luaL_checkint(L, 1);
    height = luaL_checkint(L, 2);
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
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  if (x < 0 || x >= self->width || y < 0 || y >= self->height) {
    lua_pushinteger(L, 0);
  } else {
    lua_pushinteger(L, self->data[x + y * self->width]);
  }
  return 1;
}


int l_image_setPixel(lua_State *L) {
  image_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  pixel_t color = luaL_checkint(L, 4);
  image_setPixel(self, x, y, color);
  return 0;
}


int l_image_mapPixel(lua_State *L) {
  image_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  luaL_argcheck(L, lua_isfunction(L, 2), 2, "expected function");
  int y, x;
  for (y = 0; y < self->height; y++) {
    for (x = 0; x < self->width; x++) {
      lua_pushvalue(L, 2);
      lua_pushinteger(L, x);
      lua_pushinteger(L, y);
      lua_pushinteger(L, self->data[x + y * self->width]);
      lua_call(L, 3, 1);
      self->data[x + y * self->width] = lua_tointeger(L, -1);
      lua_pop(L, 1);
    }
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
    { "mapPixel",       l_image_mapPixel      },
    { 0, 0 },
  };
  luaobj_newclass(L, CLASS_NAME, NULL, l_image_new, reg);
  return 1;
}
