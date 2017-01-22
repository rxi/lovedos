#include <string.h>
#include <dos.h>
#include <stdbool.h>
#include "mixer.h"
#include "soundblaster.h"

#define MIXER_MAX_SOUNDS 8

typedef struct {
  int offset;
  sound_t const *sound;
} mixed_sound_t;

static mixed_sound_t sounds[MIXER_MAX_SOUNDS];
static int activeSounds = 0;
static int16_t data[SOUNDBLASTER_SAMPLES_PER_BUFFER] = {0};
static bool canmix = true;

void mixer_init(void) {
}

int16_t const * mixer_getNextBlock(void) {
  canmix = true;
  return data;
}


void mixer_play(sound_t const *sound) {
  if(activeSounds == MIXER_MAX_SOUNDS) {
    // TODO Replace older sound with new one instead?
    return;
  }

  for(int i = 0; i < activeSounds; ++i) {
    if(sounds[i].sound == sound) {
      sounds[i].offset = 0;
      return;
    }
  }

  sounds[activeSounds].offset = 0;
  sounds[activeSounds].sound = sound;
  ++activeSounds;
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
  if(!canmix)
    return;

  memset(data, 0, SOUNDBLASTER_SAMPLES_PER_BUFFER);

  for(int i = 0; i < activeSounds; ++i) {
    mixed_sound_t *snd = sounds + i;
    int len = snd->sound->sampleCount;
    int16_t const* soundBuf = snd->sound->samples;

    if(len > SOUNDBLASTER_SAMPLES_PER_BUFFER) {
      len = SOUNDBLASTER_SAMPLES_PER_BUFFER;
    }

    for(int offset = 0; offset < len; ++offset) {
      data[offset] = mix(data[offset], soundBuf[offset]);
    }

    snd->offset += len;


  }


  for(int i = activeSounds-1; i >= 0; --i) {
    mixed_sound_t *snd = sounds + i;
    if(snd->offset == snd->sound->sampleCount) {
      *snd = sounds[activeSounds-1];
      --activeSounds;
    }
  }

  canmix = false;
}
