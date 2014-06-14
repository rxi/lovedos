/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef FONT_H
#define FONT_H

#include "image.h"

typedef struct {
  image_t image;
  int charSpacing, lineSpacing;
} font_t;

const char *font_init(font_t *self, const char *filename);
const char *font_initEmbedded(font_t *self);
void font_deinit(font_t *self);
void font_blit(font_t *self, pixel_t *buf, int bufw, int bufh,
               const char *str, int dx, int dy);


#endif
