/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef IMAGE_H
#define IMAGE_H

#include "vga.h"
#include "luaobj.h"


#define image_setPixel(self, x, y, color)\
  if ((x) > 0 && (x) < (self)->width && (y) > 0 && (y) < (self)->height) {\
    (self)->data[(x) + (y) * (self)->width] = (color);\
  }


enum {
  IMAGE_NORMAL,
  IMAGE_FAST,
  IMAGE_AND,
  IMAGE_OR,
  IMAGE_COLOR,
} IMAGE_BLEND_MODE;


typedef struct {
  pixel_t *data;
  pixel_t *mask;
  int width, height;
} image_t;


void image_setColor(pixel_t color);
void image_setBlendMode(int mode);
void image_setFlip(int mode);

const char *image_init(image_t *self, const char *filename);
void image_initBlank(image_t*, int, int);
void image_blit(image_t *self, pixel_t *buf, int bufw, int bufh,
                int dx, int dy, int sx, int sy, int sw, int sh);
void image_deinit(image_t*);

#endif
