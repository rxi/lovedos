#pragma once

#include <stdint.h>
#include "source.h"


void mixer_init(void);
void mixer_deinit(void);
int16_t const* mixer_getNextBlock(void);
void mixer_play(source_t const *source);
void mixer_mix(void);
