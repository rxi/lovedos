#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "sound.h"
#include "lib/dmt/dmt.h"
#define STB_VORBIS_HEADER_ONLY 1
#include "lib/stb/stb_vorbis.c"


int sound_initSilence(sound_t *self, int samples) {
  self->sampleCount = samples;
  self->samples = (int16_t*)dmt_malloc(samples * sizeof(int16_t));
  return 0;
}


char const* sound_init(sound_t *self, char const* filename) {
  short *buffer;
  int channels;
  int samplingRate;
  char const* err = NULL;

  int len = stb_vorbis_decode_filename(filename, &channels, &samplingRate, &buffer);
  if(len == -1) {
    return "could not decode Vorbis file";
  }

  if(channels != 1) {
    err = "only single channel audio files supported";
    goto fail;
  }

  if(samplingRate != 22050) {
    err = "only 22050Hz audio files suported";
    goto fail;
  }

  int bufSize = len * sizeof(int16_t);
  self->samples = (int16_t*)dmt_malloc(bufSize);
  memcpy(self->samples, buffer, bufSize);
  
fail:
  free(buffer);
  return err;
}


void sound_deinit(sound_t *self) {
  dmt_free(self->samples);
}

