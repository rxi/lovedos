#ifndef SOUND_H
#define SOUND_H


#define SOUND_ERROR_DECODE 1
#define SOUND_ERROR_CHANNEL_COUNT 2
#define SOUND_ERROR_SAMPLING_RATE 3

typedef struct  {
  int sampleCount;
  int16_t *samples;
} sound_t;

int sound_initSilence(sound_t *self, int samples);
char const* sound_init(sound_t *self, char const* filename);
void sound_deinit(sound_t *self);


#endif
