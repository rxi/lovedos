#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "source.h"
#include "lib/dmt/dmt.h"
#include "wavefile.h"


int source_initSilence(source_t *self, int samples) {
  self->sampleCount = samples;
  self->samples = (int16_t*)dmt_malloc(samples * sizeof(int16_t));
  return 0;
}


char const* source_init(source_t *self, char const* filename) {
  return wavefile_load(self, filename);
}


void source_deinit(source_t *self) {
  dmt_free(self->samples);
}

