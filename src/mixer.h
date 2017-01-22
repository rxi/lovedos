#pragma once

#include <stdint.h>
#include "sound.h"


void mixer_init(void);
void mixer_deinit(void);
int16_t const* mixer_getNextBlock(void);
void mixer_play(sound_t const *sound);
void mixer_mix(void);
