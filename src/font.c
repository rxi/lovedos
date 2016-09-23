/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/dmt/dmt.h"
#include "lib/stb/stb_truetype.h"
#include "font.h"
#include "luaobj.h"


static const char *initFont(font_t *self, const void *data, int ptsize) {
  int i;

  /* Init font */
  stbtt_fontinfo font;
  if ( !stbtt_InitFont(&font, data, 0) ) {
    return "could not load font";
  }

  /* Get height and scale */
  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
  float scale = stbtt_ScaleForMappingEmToPixels(&font, ptsize);
  self->height = (ascent - descent + lineGap) * scale;

  /* Init image */
  int w = 128, h = 128;
retry:
  image_initBlank(&self->image, w, h);

  /* Load glyphs */
  float s = stbtt_ScaleForMappingEmToPixels(&font, 1) /
            stbtt_ScaleForPixelHeight(&font, 1);
  int res = stbtt_BakeFontBitmap(
    data, 0, ptsize * s, self->image.data, w, h, 0, 128, self->glyphs);

  /* Retry with a larger image buffer if the buffer wasn't large enough */
  if (res < 0) {
    w <<= 1;
    h <<= 1;
    image_deinit(&self->image);
    goto retry;
  }

  /* Adjust glyph yoffsets */
  int scaledAscent = ascent * scale;
  for (i = 0; i < 128; i++) {
    self->glyphs[i].yoff += scaledAscent;
  }

  /* Init image data and mask */
  for (i = 0; i < w * h; i++) {
    self->image.data[i] = (self->image.data[i] > 127) ? 1 : 0;
    self->image.mask[i] = (self->image.data[i] == 0) ? 0xff : 0;
  }

  /* Return NULL for no error */
  return NULL;
}


const char *font_init(font_t *self, const char *filename, int ptsize) {
  const char *errmsg = NULL;
  void *data = NULL;
  FILE *fp = NULL;
  memset(self, 0, sizeof(*self));

  /* Load font file */
  fp = fopen(filename, "rb");
  if (!fp) {
    errmsg = "could not open font file";
    goto fail;
  }
  fseek(fp, 0, SEEK_END);
  int sz = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  data = dmt_malloc(sz);
  fread(data, sz, 1, fp);
  fclose(fp);
  fp = NULL;

  /* Init font */
  errmsg = initFont(self, data, ptsize);
  if (errmsg) {
    goto fail;
  }

  /* Free font data */
  dmt_free(data);

  return NULL;

fail:
  if (fp) fclose(fp);
  dmt_free(data);
  return errmsg;
}


const char *font_initEmbedded(font_t *self, int ptsize) {
  #include "font_embedded.c"
  return initFont(self, font_data, 8);
}


void font_deinit(font_t *self) {
  image_deinit(&self->image);
}


extern int image_blendMode;
extern int image_flip;

void font_blit(font_t *self, pixel_t *buf, int bufw, int bufh,
               const char *str, int dx, int dy
) {
  const char *p = str;
  int x = dx;
  int y = dy;

  int oldBlendMode = image_blendMode;
  int oldFlip = image_flip;
  image_blendMode = IMAGE_COLOR;
  image_flip = 0;

  while (*p) {
    stbtt_bakedchar *g = &self->glyphs[(int) (*p & 127)];
    int w = g->x1 - g->x0;
    int h = g->y1 - g->y0;
    image_blit(
      &self->image, buf, bufw, bufh,
      x + g->xoff, y + g->yoff, g->x0, g->y0, w, h);
    x += g->xadvance;
    p++;
  }

  image_blendMode = oldBlendMode;
  image_flip = oldFlip;
}



#define CLASS_TYPE  LUAOBJ_TYPE_FONT
#define CLASS_NAME  "Font"


int l_font_new(lua_State *L) {
  const char *filename = lua_isnoneornil(L, 1) ? NULL : luaL_checkstring(L, 1);
  int ptsize = luaL_optnumber(L, 2, 12);
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
