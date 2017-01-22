#include <stdint.h>
#include "sound.h"
#include "lib/dmt/dmt.h"

int sound_init(sound_t *self, int samples) {
  self->sampleCount = samples;
  self->samples = (int16_t*)dmt_malloc(samples * sizeof(int16_t));
  return 0;
}


void sound_deinit(sound_t *self) {
  dmt_free(self->samples);
}

