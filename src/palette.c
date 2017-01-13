/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <pc.h>

#include "palette.h"
#include "vga.h"

#define MAX_IDX   256
#define MAP_SIZE  1024
#define MAP_MASK  (MAP_SIZE - 1)

struct { unsigned color; int idx; } palette_map[MAP_SIZE];
unsigned palette_palette[MAX_IDX];
int palette_nextIdx;
int palette_inited;


void palette_init(void) {
  if (palette_inited) return;
  palette_reset();
  palette_inited = 1;
}


void palette_reset(void) {
  /* Reset nextIdx -- start at idx 1 as 0 is used for transparency */
  palette_nextIdx = 1;
  /* Reset palette_map */
  int i;
  for (i = 0; i < MAP_SIZE; i++) {
    palette_map[i].idx = -1;
  }
}


static unsigned hash(const void *data, int size) {
  unsigned hash = 5381;
  const unsigned char *p = data;
  while (size--) {
    hash = ((hash << 5) + hash) ^ *p++;
  }
  return hash;
}


int palette_colorToIdx(int r, int g, int b) {
  palette_init();

  /* Make 24bit rgb color */
  unsigned color = ((b  & 0xff) << 16) | ((g & 0xff) << 8) | (r & 0xff);

  /* Hash color */
  unsigned h = hash(&color, sizeof(color));

  /* Find color in hashmap */
  int i = h & MAP_MASK;
  while (palette_map[i].idx != -1) {
    if (palette_map[i].color == color) {
      return palette_map[i].idx;
    }
    i = (i + 1) & MAP_MASK;
  }

  /* Color wasn't found in hashmap -- Add to system palette and map */
  if (palette_nextIdx >= MAX_IDX) {
    return -1;  /* Return -1 for error if we've exceeded the palette capacity */
  }
  int idx = palette_nextIdx++;

  /* Update internal palette table */
  palette_palette[idx] = color;

  /* Update system palette */
  vga_setPalette(idx, r, g, b);

  /* Add to hashmap and return idx */
  palette_map[i].color = color;
  palette_map[i].idx = idx;
  return idx;
}


int palette_idxToColor(int idx, int *rgb) {
  /* Bounds check, return -1 on error */
  if (idx <= 0 || idx >= MAX_IDX) {
    return -1;
  }

  /* Store color in array */
  unsigned color = palette_palette[idx];
  rgb[0] = (color      ) & 0xff; /* r */
  rgb[1] = (color >>  8) & 0xff; /* g */
  rgb[2] = (color >> 16) & 0xff; /* b */

  /* Return 0 for ok */
  return 0;
}
