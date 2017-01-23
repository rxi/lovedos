/**
 * Copyright (c) 2017 Florian Kesseler
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include <dos.h>
#include <stdbool.h>
#include "mixer.h"
#include "soundblaster.h"

// Configure me!
#define MIXER_MAX_SOURCES 8


typedef struct {
  int offset;
  source_t const *source;
} mixed_sound_t;


static mixed_sound_t sources[MIXER_MAX_SOURCES];
static int           activeSources                         = 0;
static int16_t       data[SOUNDBLASTER_SAMPLES_PER_BUFFER] = {0};
static bool          canmix                                = true;


int16_t const * mixer_getNextBlock(void) {
  canmix = true;
  return data;
}


void mixer_play(source_t const *source) {
  if(activeSources == MIXER_MAX_SOURCES) {
    // TODO Replace older source with new one instead?
    return;
  }

  for(int i = 0; i < activeSources; ++i) {
    if(sources[i].source == source) {
      sources[i].offset = 0;
      return;
    }
  }

  sources[activeSources].offset = 0;
  sources[activeSources].source = source;
  ++activeSources;
}


static inline int16_t mix(int32_t a, int32_t b) {
  int32_t res = a + b;
  if(res > INT16_MAX)
    res = INT16_MAX;
  if(res < INT16_MIN)
    res = INT16_MIN;
  return res;
}


void mixer_mix(void) {
  if(!canmix) {
    return;
  }

  memset(data, 0, SOUNDBLASTER_SAMPLES_PER_BUFFER * sizeof(int16_t));

  for(int i = 0; i < activeSources; ++i) {
    mixed_sound_t *snd = sources + i;
    int len = snd->source->sampleCount - snd->offset;
    int16_t const* sourceBuf = snd->source->samples + snd->offset;

    if(len > SOUNDBLASTER_SAMPLES_PER_BUFFER) {
      len = SOUNDBLASTER_SAMPLES_PER_BUFFER;
    }

    for(int offset = 0; offset < len; ++offset) {
      data[offset] = mix(data[offset], sourceBuf[offset]);
    }

    snd->offset += len;
  }

  for(int i = activeSources-1; i >= 0; --i) {
    mixed_sound_t *snd = sources + i;
    if(snd->offset == snd->source->sampleCount) {
      *snd = sources[activeSources-1];
      --activeSources;
    }
  }

  canmix = false;
}
