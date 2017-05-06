#include "lib/cmixer/cmixer.h"
#include "soundblaster.h"


static short audio_buffer[SOUNDBLASTER_SAMPLES_PER_BUFFER * 2];

static const short* audio_callback(void) {
  /* For the moment the soundblaster code expects mono audio while the cmixer
  ** library outputs stereo -- we process to a stereo buffer, then copy the left
  ** channel to the start of the buffer */
  int i;
  int len = SOUNDBLASTER_SAMPLES_PER_BUFFER;
  cm_process(audio_buffer, len * 2);
  for (i = 0; i < len; i++) {
    audio_buffer[i] = audio_buffer[i * 2];
  }
  return audio_buffer;
}


void audio_init(void) {
  cm_init(soundblaster_getSampleRate());
  soundblaster_init(audio_callback);
}


void audio_deinit(void) {
  soundblaster_deinit();
}
