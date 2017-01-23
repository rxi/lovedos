/**
 * Copyright (c) 2017 Florian Kesseler
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "source.h"
#include "filesystem.h"
#include "lib/dmt/dmt.h"
#include "wav.h"


char const* source_init(source_t *self, char const* filename) {
  int fileSize;
  void *waveData = filesystem_read(filename, &fileSize);

  char const *err = NULL;
  wav_t wav;
  int res = wav_read(&wav, waveData, fileSize);

  if(res != WAV_ESUCCESS) {
    err = wav_strerror(res);
    goto fail;
  }

  if(wav.bitdepth != 16) {
    err = "Invalid Audio Format (only 16 Bit supported)";
    goto fail;
  }

  if(wav.samplerate != 22050) {
    err = "Invalid Audio Format (only 22050Hz supported)";
    goto fail;
  }

  if(wav.channels != 1) {
    err = "Invalid Audio Format (only mono supported)";
    goto fail;
  }

  self->sampleCount = wav.length;
  self->data = waveData;
  self->samples = (int16_t*)wav.data;

  return NULL;

fail:
  filesystem_free(waveData);
  return err;
}


void source_deinit(source_t *self) {
  filesystem_free(self->data);
}

