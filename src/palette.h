/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef PALETTE_H
#define PALETTE_H

void palette_init(void);
void palette_reset(void);
int palette_colorToIdx(int r, int g, int b);
int palette_idxToColor(int idx, int *rgb);

#endif
