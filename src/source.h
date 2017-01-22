/**
 * Copyright (c) 2017 rnlf
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef SOURCE_H
#define SOURCE_H

#include <stdint.h>

typedef struct  {
  int sampleCount;
  int16_t *samples;
} source_t;

int source_initSilence(source_t *self, int samples);
char const* source_init(source_t *self, char const* filename);
void source_deinit(source_t *self);

#endif
