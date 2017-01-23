/**
 * Copyright (c) 2017 Florian Kesseler
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef WAVEFILE_H
#define WAVEFILE_H

#include "source.h"

char const* wavefile_load(source_t *source, char const *filename);

#endif
