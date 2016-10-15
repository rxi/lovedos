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
#include "lib/stb/stb_image.h"
#include "filesystem.h"
#include "image.h"
#include "palette.h"

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
  void *filedata = NULL;
  unsigned char *data32 = NULL;
  memset(self, 0, sizeof(*self));

  /* Load file data */
  int size;
  filedata = filesystem_read(filename, &size);
  if (!filedata) {
    errmsg = "could not read file";
    goto fail;
  }

  /* Load 32bit image data */
  int width, height, n;
  data32 = stbi_load_from_memory(filedata, size, &width, &height, &n, 4);
  if (!data32) {
    errmsg = "could not load image file";
    goto fail;
  }

  /* Free file data */
  filesystem_free(filedata);
  filedata = NULL;

  /* Set dimensions and allocate memory */
  int sz = width * height;
  self->width = width;
  self->height = height;
  self->data = dmt_malloc(sz);

  /* Load pixels into struct, converting 32bit to 8bit paletted */
  int i;
  for (i = 0; i < width * height; i++) {
    unsigned char *p = data32 + i * 4;
    int r = p[0];
    int g = p[1];
    int b = p[2];
    int a = p[3];
    int idx = palette_colorToIdx(r, g, b);
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
  filesystem_free(filedata);
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
