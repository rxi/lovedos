#ifndef SOUND_H
#define SOUND_H


typedef struct  {
  int sampleCount;
  int16_t *samples;
} sound_t;

int sound_init(sound_t *self, int samples);
void sound_deinit(sound_t *self);


#endif
