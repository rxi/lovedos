/** 
 * Copyright (c) 2015 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */


#ifndef WAV_H
#define WAV_H

#include <stdlib.h>
#include <stdint.h>

typedef struct {
  const void *data;
  int bitdepth;
  int samplerate;
  int channels;
  size_t length;
} wav_t;

enum {
  WAV_ESUCCESS    =  0,
  WAV_EFAILURE    = -1,
  WAV_EBADHEADER  = -2,
  WAV_EBADFMT     = -3,
  WAV_ENOFMT      = -4,
  WAV_ENODATA     = -5,
  WAV_ENOSUPPORT  = -6
};

int wav_read(wav_t *w, const void *data, size_t len);
const char *wav_strerror(int err);

#endif
