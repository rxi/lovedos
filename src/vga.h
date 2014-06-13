/** 
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef VGA_H
#define VGA_H

#define VGA_WIDTH   320
#define VGA_HEIGHT  200

typedef unsigned char pixel_t;

void vga_init(void);
void vga_deinit(void);
void vga_update(pixel_t *buffer);

#endif
