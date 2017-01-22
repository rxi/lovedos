/**
 * Copyright (c) 2017 rnlf
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef MIXER_H
#define MIXER_H

#include <stdint.h>
#include "source.h"

void mixer_init(void);
void mixer_deinit(void);
int16_t const* mixer_getNextBlock(void);
void mixer_play(source_t const *source);
void mixer_mix(void);

#endif
