/**
 * Copyright (c) 2017 Florian Kesseler
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef SOUNDBLASTER_H
#define SOUNDBLASTER_H

#include <stdint.h>

// Error codes
#define SOUNDBLASTER_ENV_NOT_SET 1
#define SOUNDBLASTER_ENV_INVALID 2
#define SOUNDBLASTER_DOS_ERROR   3
#define SOUNDBLASTER_RESET_ERROR 4
#define SOUNDBLASTER_ALLOC_ERROR 5

#define SOUNDBLASTER_SAMPLES_PER_BUFFER 2048

typedef int16_t const* (*soundblaster_getSampleProc)(void);

int soundblaster_init(soundblaster_getSampleProc sampleproc);
void soundblaster_deinit(void);

#endif
