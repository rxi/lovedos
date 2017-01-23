/**
 * Copyright (c) 2017 rnlf
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "source.h"
#include "lib/dmt/dmt.h"
#include "wavefile.h"


char const* source_init(source_t *self, char const* filename) {
  return wavefile_load(self, filename);
}


void source_deinit(source_t *self) {
  dmt_free(self->samples);
}

